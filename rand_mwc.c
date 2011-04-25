/*
 * rand_mwc.c  Copyright 2011 Andrew Schaumberg
 *
 * Multiply-with-carry pseudo-random number generator.
 */

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "gc.h"
#include "main.h"

int rand_mwc(int datalen)
{
  unsigned int mw, mz;
  int ret, i, j, t, p, f, passed,
      databytes, showlen, showbytes;
  int *in, *out, *passes, *fails;

  size_t global, local; /* memory sizes for gpu calculations */

  cl_device_id     devid;
  cl_device_type   devtype;
  cl_platform_id   platform = NULL;
  cl_platform_id*  platforms;
  cl_uint          platformslen;
  cl_context       context;
  cl_command_queue cmdq;
  cl_program       prog;
  cl_kernel        kern;

  cl_mem gpuin, gpuout; /* device memory for input and output arys */

  int gpu = 1; /* use gpu */
  char *platform_buf;
  int platform_bufsize = 128;
  char *progerr_buf;
  size_t progerr_bufsize = 2048, progerr_buflen;

  /* For reading OpenCL program source */
  FILE *fp;
  struct stat info;
  char *source;
  char *sourcefn = "rand_mwc.cl";
  off_t sourcelen;
  size_t sourceread;

  ret = -1;
  databytes = sizeof(int) * datalen;

  /***
   * Allocate buffers, read OpenCL program source, setup input.
   */

  /* Show this many results each test */
  showlen   = (int)log((double)datalen);
  showbytes = sizeof(int) * showlen;

  allocreturn(in,     databytes);
  allocreturn(out,    databytes);
  allocreturn(passes, showbytes);
  allocreturn(fails,  showbytes);
  allocreturn(platform_buf, platform_bufsize);

  ret = stat(sourcefn, &info);
  if (ret) {
    E("Could not get status of file %s, ret %d, err %d", sourcefn, ret, errno);
    return -1;
  }
  sourcelen = info.st_size; /* filesize of source */
  if (sourcelen <= 0) {
    E("File %s length of %ld invalid!", sourcefn, sourcelen);
    return -1;
  }

  fp = fopen(sourcefn, "r");
  if (!fp) {
    E("Failed to read program source file %s, err %d", sourcefn, errno);
    return -1;
  }
  alloclongreturn(source, sourcelen);
  sourceread = fread(source, 1, sourcelen, fp);
  if (sourceread < sourcelen) {
    E("Program source read of %s returned %d bytes!  Source is %ld bytes.",
      sourcefn, sourceread, sourcelen);
    return -1; /* Cannot expect to continue */
  }

  /* Input for GPU, input cannot be 0 */
  for (i = 1; i <= datalen; i++) {
    in[i] = i;
  }

  /***
   * Setup GPU
   */

  /* Get platform before getdeviceids
   * http://developer.amd.com/Support/KnowledgeBase/Lists/KnowledgeBase/DispForm.aspx?ID=71
   */
  ret = clGetPlatformIDs(0, NULL, &platformslen);
  if (ret != CL_SUCCESS) {
    E("clGetPlatformIDs returned %d, output num platforms %d.",
      ret, platformslen);
    return -1;
  }
  if (platformslen <= 0) {
    E("Number of OpenCL platform IDs is %d!", platformslen);
    return -1;
  }

  allocreturn(platforms, sizeof(cl_platform_id) * platformslen);
  ret = clGetPlatformIDs(platformslen, platforms, NULL);
  if (ret != CL_SUCCESS) {
    E("clGetPlatformIDs returned %d, given num platforms %d",
      ret, platformslen);
    return -1;
  }
  for (i = 0; i < platformslen; i++) {
    ret = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, platform_bufsize,
                            platform_buf, NULL);
    I("clGetPlatformInfo[%d] returned %d platform %s", i, ret, platform_buf);
    if (ret == CL_SUCCESS) {
      /* TBD: allow use of more than one available platform. */
      platform = platforms[i];
    }
  }

  devtype = gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU;
  ret = clGetDeviceIDs(platform, devtype, 1, &devid, NULL);
  if (ret != CL_SUCCESS) {
    E("Device group creation failed %d", ret);
    return -1;
  }

  if (platform != NULL) {
    /* Platform found, use it. */
    cl_context_properties cps[3] = {
      CL_CONTEXT_PLATFORM, 
      (cl_context_properties)platform, 
      0
    };
    context = clCreateContextFromType(cps, devtype, NULL, NULL, &ret);
    if (context) {
      if (ret != CL_SUCCESS) {
        I("Type-based compute context creation returned %d", ret);
      }
    } else {
      E("Type-based compute context creation failed %d!", ret);
      /* Fall through to default context creation method */
    }
  }
  if (platform == NULL || context == NULL) {
    /* Use default platform.  Tends to fail. */
    context = clCreateContext(0, 1, &devid, NULL, NULL, &ret);
    if (context) {
      if (ret != CL_SUCCESS) {
        I("Default compute context creation returned %d", ret);
      }
    } else {
      E("Default compute context creation failed %d!", ret);
      return -1;
    }
  }

  /***
   * Build GPU program
   */

  cmdq = clCreateCommandQueue(context, devid, 0, &ret);
  if (cmdq) {
    if (ret != CL_SUCCESS) {
      I("Command queue creation returned %d", ret);
    }
  } else {
    E("Command queue creation failed %d!", ret);
    return -1;
  }

  prog = clCreateProgramWithSource(context, 1, (const char **)&source, NULL,
                                   &ret);
  if (!prog) {
    E("Program creation failed %d!", ret);
    return -1;
  } else if (ret != CL_SUCCESS) {
    I("Program creation returned %d.", ret);
  }

  ret = clBuildProgram(prog, 0, NULL, NULL, NULL, NULL);
  if (ret != CL_SUCCESS) {
    E("Program build failed %d!", ret);
    allocreturn(progerr_buf,  progerr_bufsize);
    clGetProgramBuildInfo(prog, devid, CL_PROGRAM_BUILD_LOG, progerr_bufsize,
                          progerr_buf, &progerr_buflen);
    E("Build info follows:\n%s", progerr_buf);
    return -1;
  }

  kern = clCreateKernel(prog, "rand_mwc", &ret);
  if (!kern || ret != CL_SUCCESS) {
    E("Kernel creation failed %d!", ret);
    return -1;
  }

  gpuin = clCreateBuffer(context,  CL_MEM_READ_ONLY,  databytes, NULL, &ret);
  if (!gpuin) {
    E("Input buffer creation on device failed %d!", ret);
    return -1;
  } else if (ret != CL_SUCCESS) {
    I("Input buffer creation on device returned %d", ret);
  }
  gpuout = clCreateBuffer(context, CL_MEM_WRITE_ONLY, databytes, NULL, &ret);
  if (!gpuout) {
    E("Output buffer creation on device failed %d!", ret);
    return -1;
  } else if (ret != CL_SUCCESS) {
    I("Output buffer creation on device returned %d", ret);
  }

  ret = clEnqueueWriteBuffer(cmdq, gpuin, CL_TRUE, 0, databytes, in, 0, NULL,
                             NULL);
  if (ret != CL_SUCCESS) {
    E("Buffer write failed %d!", ret);
    return -1;
  }

#define kernarg(kern, idx, var, size) do { \
  ret = clSetKernelArg(kern, idx, size, var); \
  if (ret != CL_SUCCESS) { \
    E("Kernel argument %d set failed %d!", idx, ret); \
    return -1; \
  } \
} while(0);
  kernarg(kern, 0, &gpuin,  sizeof(cl_mem));
  kernarg(kern, 1, &gpuout, sizeof(cl_mem));
  kernarg(kern, 2, &databytes, sizeof(int));

  ret = clGetKernelWorkGroupInfo(kern, devid, CL_KERNEL_WORK_GROUP_SIZE,
                                 sizeof(local), &local, NULL);
  if (ret != CL_SUCCESS) {
      E("Max work group size retrieval failed %d!", ret);
      return -1;
  }

  /***
   * Run GPU kernel, read output, then release resources.
   */

  global = databytes;
  ret = clEnqueueNDRangeKernel(cmdq, kern, 1, NULL, &global, &local, 0, NULL,
                               NULL);
  if (ret != CL_SUCCESS) {
      E("Kernel exec failed %d!", ret);
      return -1;
  }

#define clreturn(func, arg) do { \
  ret = func(arg); \
  if (ret != CL_SUCCESS) { \
    E("%s error %d!", #func, ret); \
    return -1; \
  } \
} while (0);
  clreturn(clFlush,  cmdq);
  clreturn(clFinish, cmdq); /* Blocks for cmdq to complete */

  ret = clEnqueueReadBuffer(cmdq, gpuout, CL_TRUE, 0, databytes, out, 0, NULL, NULL );
  if (ret != CL_SUCCESS) {
    E("Output read failed %d!", ret);
    return -1;
  }

  clreturn(clReleaseKernel,       kern);
  clreturn(clReleaseProgram,      prog);
  clreturn(clReleaseMemObject,    gpuin);
  clreturn(clReleaseMemObject,    gpuout);
  clreturn(clReleaseCommandQueue, cmdq);
  clreturn(clReleaseContext,      context);

  /***
   * Validate results, print report
   */

  printf("rand multiply-with-carry test: ");
  f = p = passed = 0;
  for (i = 0; i < datalen; i += 2) {
    mw = in[i];
    mz = in[i+1];

    mw = 18000 * (mw & 0xffff) + (mw >> 16);
    mz = 36969 * (mz & 0xffff) + (mz >> 16);
    t = (mz << 16) + mw; /* first generated random */
    j = i;
    if (out[j] == t) {
      passed++;
      if (p < showlen) {
        passes[p++] = j;
      }
    } else {
      if (f < showlen) {
        fails[f++] = j;
      }
    }

    mw = 18000 * (mw & 0xffff) + (mw >> 16);
    mz = 36969 * (mz & 0xffff) + (mz >> 16);
    t = (mz << 16) + mw; /* second generated random */
    j++;
    if (out[j] == t) {
      passed++;
      if (p < showlen) {
        passes[p++] = j;
      }
    } else {
      if (f < showlen) {
        fails[f++] = j;
      }
    }
  }
  printf("%d/%d passed\n", passed, datalen);

  if (p > 0) {
    printf("Some passes:\n");
    for (i = 0; i < p; i++) {
      t = passes[i];
      printf("in[%4d] = %11d\tout[%4d] = %11d\n",
             t, in[t], t, out[t]);
    }
  }
  if (f > 0) {
    printf("Some fails:\n");
    for (i = 0; i < f; i++) {
      t = fails[i];
      printf("in[%4d] = %11d\tout[%4d] = %11d\n",
             t, in[t], t, out[t]);
    }
  }

  if (passed == datalen) {
    return 0;
  }
  return -1;
}

