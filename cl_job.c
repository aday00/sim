/*
 * cl_job.c  Copyright 2011 Andrew Schaumberg
 *
 * Simple interface to setup, execute, and teardown an OpenCL job.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cl_job.h"
#include "cl_job_private.h"
#include "gc.h"
#include "main.h"
#include "queue.h"

/* Describes available GPU hardware and the particular h/w in use */
typedef struct clhw_s {
  cl_device_id     devid;
  cl_device_type   devtype;
  cl_platform_id   platform;
  cl_uint          platformslen;
  cl_platform_id  *platforms;
} clhw_t;
clhw_t *clhw;

/* Describes the non-dynamic software state of GPU */
typedef struct clsw_s {
  cl_context       context;
  cl_command_queue cmdq;
  cl_uint          kargc; /* count of kernel arguments */
} clsw_t;
clsw_t *clsw; /* TBD: cljob_t member should point at clsw, in turn at a clhw */

inline _cljob_t *cljob_from_ticket(cljob_ticket t)
{
  return (_cljob_t *) t; /* Trivially convert a ticket to a job */
}

/* Elements of the singly-linked list of jobs are of this type */
typedef struct jobentry_s {
  cljob_ticket            job;
  SLIST_ENTRY(jobentry_s) entry; /* entry points to next jobentry in list */
} jobentry_t;
/* All jobs maintained in this singly-linked list */
typedef SLIST_HEAD(jobhead_s, jobentry_s) jobhead_t;
static jobhead_t *jobhead;

int _clinithw(void)
{
  int ret, i;
  int gpu = 1; /* use gpu opencl, not cpu opencl */
  char *platform_buf;
  int platform_bufsize = 128;

  /* Get platform before getdeviceids
   * http://developer.amd.com/Support/KnowledgeBase/Lists/KnowledgeBase/DispForm.aspx?ID=71
   */
  ret = clGetPlatformIDs(0, NULL, &clhw->platformslen);
  if (ret != CL_SUCCESS) {
    E("clGetPlatformIDs returned %d, output num platforms %d.",
      ret, clhw->platformslen);
    return -1;
  }
  if (clhw->platformslen <= 0) {
    E("Number of OpenCL platform IDs is %d!", clhw->platformslen);
    return -1;
  }

  allocreturn(clhw->platforms, sizeof(cl_platform_id) * clhw->platformslen);
  ret = clGetPlatformIDs(clhw->platformslen, clhw->platforms, NULL);
  if (ret != CL_SUCCESS) {
    E("clGetPlatformIDs returned %d, given num platforms %d",
      ret, clhw->platformslen);
    return -1;
  }
  allocreturn(platform_buf, platform_bufsize);
  for (i = 0; i < clhw->platformslen; i++) {
    ret = clGetPlatformInfo(clhw->platforms[i], CL_PLATFORM_VENDOR, platform_bufsize,
                            platform_buf, NULL);
    I("clGetPlatformInfo[%d] returned %d platform %s", i, ret, platform_buf);
    if (ret == CL_SUCCESS) {
      /* TBD: allow use of more than one available platform. */
      clhw->platform = clhw->platforms[i];
    }
  }

  /* TBD: provide interface to toggle "gpu" variable */
  clhw->devtype = gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU;
  ret = clGetDeviceIDs(clhw->platform, clhw->devtype, 1, &clhw->devid, NULL);
  if (ret != CL_SUCCESS) {
    E("Device group creation failed %d", ret);
    return -1;
  }

  return 0;
}
int _clinitsw(void)
{
  int ret;

  if (clhw->platform != NULL) {
    /* Platform found, use it. */
    cl_context_properties cps[3] = {
      CL_CONTEXT_PLATFORM, 
      (cl_context_properties)clhw->platform, 
      0
    };
    clsw->context = clCreateContextFromType(cps, clhw->devtype, NULL, NULL, &ret);
    if (clsw->context) {
      if (ret != CL_SUCCESS) {
        I("Type-based compute context creation returned %d", ret);
      }
    } else {
      E("Type-based compute context creation failed %d!", ret);
      /* Fall through to default context creation method */
    }
  }
  if (clhw->platform == NULL || clsw->context == NULL) {
    /* Use default platform.  Tends to fail. */
    clsw->context = clCreateContext(0, 1, &clhw->devid, NULL, NULL, &ret);
    if (clsw->context) {
      if (ret != CL_SUCCESS) {
        I("Default compute context creation returned %d", ret);
      }
    } else {
      E("Default compute context creation failed %d!", ret);
      return -1;
    }
  }

  clsw->cmdq = clCreateCommandQueue(clsw->context, clhw->devid, 0, &ret);
  if (clsw->cmdq) {
    if (ret != CL_SUCCESS) {
      I("Command queue creation returned %d", ret);
    }
  } else {
    E("Command queue creation failed %d!", ret);
    return -1;
  }

  return 0;
}

/* Call this once at program startup */
int clinit(void)
{
  int ret;

  /* Allocate descriptors and linked list to manage jobs. */
  zallocreturn(clhw, sizeof(clhw_t));
  zallocreturn(clsw, sizeof(clsw_t));

  allocreturn(jobhead, sizeof(jobhead_t));
  SLIST_INIT(jobhead);

  /* Probe for hardware */
  callreturn(_clinithw());

  /* Now that hardware state is defined, set up software state */
  callreturn(_clinitsw());

  return 0;
}

/* Initialize state for this new job and push to job list */
int clregister(cljob_ticket *jobptr)
{
  jobentry_t *je;
  _cljob_t   *jp;
  bufhead_t  *bh;

  allocreturn( je, sizeof(jobentry_t));
  zallocreturn(jp, sizeof(_cljob_t));
  allocreturn( bh, sizeof(bufhead_t));
  SLIST_INIT(bh);

  jp->bufhead = bh;
  *jobptr     = jp;
  je->job     = jp;

  SLIST_INSERT_HEAD(jobhead, je, entry);
  return 0;
}

/* Build source from file named sourcefn, returning sourcelen and source.
 */
int _clreadprogramsource(const char *sourcefn, off_t *sourcelen, char **source)
{
  /* For reading OpenCL program source */
  FILE *fp;
  struct stat info;
  char *src;
  off_t srclen;
  size_t srcread;
  int ret;

  ret = stat(sourcefn, &info);
  if (ret) {
    E("Could not get status of file %s, ret %d, err %d", sourcefn, ret, errno);
    return -1;
  }
  srclen = info.st_size; /* filesize of source */
  if (srclen <= 0) {
    E("File %s length of %ld invalid!", sourcefn, srclen);
    return -1;
  }

  fp = fopen(sourcefn, "r");
  if (!fp) {
    E("Failed to read program source file %s, err %d", sourcefn, errno);
    return -1;
  }
  alloclongreturn(src, srclen);
  srcread = fread(src, 1, srclen, fp);
  if (srcread < srclen) {
    E("Program source read of %s returned %d bytes!  Source is %ld bytes.",
      sourcefn, srcread, srclen);
    return -1; /* Cannot expect to continue */
  }
  I("Source in %s follows:\n%s", sourcefn, src);

  *sourcelen = srclen;
  *source    = src;
  return 0;
}
int clbuild(cljob_ticket job, const char *sourcefn, const char *kernelfunc)
{
  char *source;
  off_t sourcelen;

  _cljob_t *jp;

  char *progerr_buf;
  size_t progerr_bufsize = 2048, progerr_buflen;

  int ret;

  ret = _clreadprogramsource(sourcefn, &sourcelen, &source);
  if (ret != 0) {
    E("_clreadprogramsource returned error %d\n", ret);
    return -1;
  } 

  /***
   * Build GPU program
   */
  jp = cljob_from_ticket(job);

  jp->prog = clCreateProgramWithSource(clsw->context, 1, (const char **)&source,
                                   NULL, &ret);
  if (!jp->prog) {
    E("Program creation failed %d!", ret);
    return -1;
  } else if (ret != CL_SUCCESS) {
    I("Program creation returned %d.", ret);
  }

  /* For #include to work, must specify -I option for build */
  ret = clBuildProgram(jp->prog, 0, NULL, "-I ./", NULL, NULL);
  if (ret != CL_SUCCESS) {
    E("Program build failed %d!", ret);
    allocreturn(progerr_buf, progerr_bufsize);
    clGetProgramBuildInfo(jp->prog, clhw->devid, CL_PROGRAM_BUILD_LOG,
                          progerr_bufsize, progerr_buf, &progerr_buflen);
    E("Build info follows:\n%s", progerr_buf);
    return -1;
  }

  jp->kern = clCreateKernel(jp->prog, kernelfunc, &ret);
  if (!jp->kern || ret != CL_SUCCESS) {
    E("Kernel creation failed %d!", ret);
    return -1;
  }


  return 0;
}

inline _clbuf_t *clbuf_from_ticket(clbuf_ticket t)
{
  return (_clbuf_t *) t; /* Trivially convert a ticket to a buf */
}


#define kernargreturn(kern, idx, size, var) do { \
  ret = clSetKernelArg(kern, idx, size, var); \
  if (ret != CL_SUCCESS) { \
    E("Kernel argument %d set failed %d!", idx, ret); \
    return -1; \
  } \
} while (0);
/* A compute kernel argument is a scalar (i.e. int, float, etc)
 * scalarp points to the scalar's value of size scalarbytes.
 */
int clkargscalar(cljob_ticket job, size_t scalarbytes, const void *scalarp)
{
  int ret;
  _cljob_t *jp;
  cl_uint   kargc;

  jp = cljob_from_ticket(job);

  kargc = clsw->kargc;
  clsw->kargc = kargc + 1;
  kernargreturn(jp->kern, kargc, scalarbytes, scalarp);

  return 0;
}
/* Append a new kernel argument for a memory buffer.
 * Reads and writes to this buffer are allowed through the clbuf_ticket.
 */
int clkargbuf(cljob_ticket job, cl_mem_flags flags, size_t hostbytes,
              void *hostmem, cl_bool blocking, clbuf_ticket *buf)
{
  int ret;

  _cljob_t *jp;
  _clbuf_t *bp;
  bufentry_t *be;
  cl_mem    devmem;
  cl_uint   kargc;

//#define dbgbuf 1
#ifdef dbgbuf
  int *h, i;
#endif

  allocreturn(bp, sizeof(_clbuf_t));

  jp = cljob_from_ticket(job);

  devmem = clCreateBuffer(clsw->context, flags, hostbytes, NULL, &ret);
  if (!devmem) {
    E("Input buffer creation on device failed %d!", ret);
    return -1;
  } else if (ret != CL_SUCCESS) {
    I("Input buffer creation on device returned %d", ret);
  }
  bp->devmem    = devmem;
  bp->hostbytes = hostbytes;
  bp->hostmem   = hostmem;
  bp->blocking  = blocking;
  bp->jobptr    = jp;

#ifdef dbgbuf
  h = hostmem;
    for(i = 0; i < 100; i++) {
      h[i] = -i;
    }
#endif

  kargc = clsw->kargc;
  clsw->kargc = kargc + 1;
  kernargreturn(jp->kern, kargc, sizeof(cl_mem), &bp->devmem);

  allocreturn(be, sizeof(bufentry_t));
  SLIST_INSERT_HEAD(jp->bufhead, be, entry);
  be->buf = bp;

  *buf = bp;
  return 0;
}

/* Enqueue a write buffer */
int clkargbufw(clbuf_ticket buf)
{
  int ret;

  _cljob_t *jp;
  _clbuf_t *bp;
  bufentry_t *be;

  bp = clbuf_from_ticket(buf);
  jp = bp->jobptr;

  ret = clEnqueueWriteBuffer(clsw->cmdq, bp->devmem, bp->blocking, 0,
                             bp->hostbytes, bp->hostmem, 0, NULL, NULL);
  if (ret != CL_SUCCESS) {
    E("Buffer write failed %d!", ret);
    return -1;
  }

  /* Place buffer on list of buffers to release on job teardown */
  allocreturn(be, sizeof(bufentry_t));
  SLIST_INSERT_HEAD(jp->bufhead, be, entry);
  return 0;
}
/* Enqueue a read buffer */
int clkargbufr(clbuf_ticket buf)
{
  int ret;

  _cljob_t *jp;
  _clbuf_t *bp;

  bp = clbuf_from_ticket(buf);
  jp = bp->jobptr;

  ret = clEnqueueReadBuffer(clsw->cmdq, bp->devmem, bp->blocking, 0,
                            bp->hostbytes, bp->hostmem, 0, NULL, NULL);
  if (ret != CL_SUCCESS) {
    E("Buffer read failed %d!", ret);
    return -1;
  }
  return 0;
}

int clmemory(cljob_ticket job, size_t globalbytes)
{
  int ret;
  size_t localbytes; /* memory sizes for gpu calculations */

  _cljob_t *jp;
  jp = cljob_from_ticket(job);

  ret = clGetKernelWorkGroupInfo(jp->kern, clhw->devid, CL_KERNEL_WORK_GROUP_SIZE,
                                  sizeof(localbytes), &localbytes, NULL);
  if (ret != CL_SUCCESS) {
    E("Max work group size retrieval failed %d!", ret);
    return -1;
  }

  jp->global = globalbytes;
  jp->local  = localbytes;
  return 0;
}
int clkernel(cljob_ticket job)
{
  int ret;
  int g, l, i, j;
  g = 4096;
  l = 1;


  _cljob_t *jp;
  bufentry_t *be;
  struct _clbuf_s         *buf;
  int *h;

  jp = cljob_from_ticket(job);

  ret = clEnqueueNDRangeKernel(clsw->cmdq, jp->kern, 1, NULL,
                               &jp->global, &jp->local, 0, NULL, NULL);
  if (ret != CL_SUCCESS) {
      E("Kernel exec failed %d!", ret);
      return -1;
  }

  return 0;
}

#define clreturn(func, arg) do { \
  ret = func(arg); \
  if (ret != CL_SUCCESS) { \
    E("%s error %d!", #func, ret); \
    return -1; \
  } \
} while (0);
int clunregister(cljob_ticket job)
{
  int ret;
  _cljob_t *jp;
  bufentry_t *be;

  jp = cljob_from_ticket(job);

  clreturn(clFlush,  clsw->cmdq); /* Issue enqueued commands */
  clreturn(clFinish, clsw->cmdq); /* Block until all commands complete */

  clreturn(clReleaseKernel,       jp->kern);
  clreturn(clReleaseProgram,      jp->prog);

  SLIST_FOREACH(be, jp->bufhead, entry) {
    clreturn(clReleaseMemObject, be->buf->hostmem);
  }

  return 0;
}

/* Calling this shouldn't be neccessary due to garbage collection */
/* XXX: support more than one clsw, have per-slwx teardown, not here. */
int clexit(void)
{
  int ret;

  if (jobhead != NULL) {
    gfree(jobhead);
  }
  if (clsw != NULL) {
    gfree(clsw);
  }
  if (clhw != NULL) {
    gfree(clhw);
  }

  clreturn(clReleaseCommandQueue, clsw->cmdq);
  clreturn(clReleaseContext,      clsw->context);
}
