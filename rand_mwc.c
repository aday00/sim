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
#include "rand_mwc.h"

int rand_mwc(int datalen)
{
  unsigned int u, v, num_generators, rands_per_generator;

  int ret, i, j, t, p, f, mwc_len, mwc_bytes, passed,
      databytes, showlen, showbytes;
  int *out, *cpuout;
  char **passes, **fails;
  char *buf, **buf_to;
  size_t bufsize = 512;
  mwc_t *in, *in_orig, m;

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

  rands_per_generator = 2; /* Each pRNG creates this many randoms */
  num_generators = datalen / rands_per_generator;
  mwc_len = num_generators;
  mwc_bytes = sizeof(mwc_t) * mwc_len;

  ret = -1;
  databytes = sizeof(int) * datalen;

  /***
   * Allocate buffers, read OpenCL program source, setup input.
   */

  /* Show this many results each test */
  showlen   = (int)log((double)datalen);
  //showbytes = sizeof(int) * showlen;
  showbytes = sizeof(void *) * showlen;

  allocreturn(in,     mwc_bytes);
  allocreturn(in_orig,mwc_bytes);
  allocreturn(out,    databytes);
  allocreturn(cpuout, databytes);
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
  for (u = 0; u <= mwc_len; u++ ) {
    v = u * rands_per_generator;
    m = (mwc_t) { v+1, v+2 };
    in[u] = m;
    in_orig[u] = m;

    mwc_next(m);
    t = mwc_rand(m);
    cpuout[v] = t;

    mwc_next(m);
    t = mwc_rand(m);
    cpuout[v+1] = t;
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

  /* For #include to work, must specify -I option for build */
  ret = clBuildProgram(prog, 0, NULL, "-I ./", NULL, NULL);
  if (ret != CL_SUCCESS) {
    E("Program build failed %d!", ret);
    allocreturn(progerr_buf, progerr_bufsize);
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

  gpuin = clCreateBuffer(context,  CL_MEM_READ_WRITE, mwc_bytes, NULL, &ret);
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

  ret = clEnqueueWriteBuffer(cmdq, gpuin, CL_TRUE, 0, mwc_bytes, in, 0, NULL,
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

  ret = clGetKernelWorkGroupInfo(kern, devid, CL_KERNEL_WORK_GROUP_SIZE,
                                 sizeof(local), &local, NULL);
  if (ret != CL_SUCCESS) {
      E("Max work group size retrieval failed %d!", ret);
      return -1;
  }

  /***
   * Run GPU kernel, read output, then release resources.
   */

  global = databytes / 2;
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
  clreturn(clFlush,  cmdq); /* Issue enqueued commands */
  clreturn(clFinish, cmdq); /* Block until all commands complete */

  /* Read results */
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
  buf_to = NULL;
  for (j = 0; j < datalen; j++) {
    if (cpuout[j] == out[j]) {
      passed++;
      if (p < showlen) {
        buf_to = &passes[p++];
      }
    } else {
      if (f < showlen) {
        buf_to = &fails[f++];
      }
    }
    if (buf_to != NULL) {
      i = j / rands_per_generator;
      m = in[i];
      allocreturn(buf, bufsize);
      if (j % 2 == 0) {
        snprintf(buf, bufsize,
          "in[%4d] = {%4d,%4d}\tcpuout[(%4d),%4d] = (%11d),%11d\tgpuout[(%4d),%4d] = (%11d),%11d",
          i, m.w, m.z, j, j+1, cpuout[j], cpuout[j+1], j, j+1, out[j], out[j+1]);
      } else {
        snprintf(buf, bufsize,
          "in[%4d] = {%4d,%4d}\tcpuout[%4d,(%4d)] = %11d,(%11d)\tgpuout[%4d,(%4d)] = %11d,(%11d)",
          i, m.w, m.z, j-1, j, cpuout[j-1], cpuout[j], j-1, j, out[j-1], out[j]);
      }
      *buf_to = buf;
      buf_to = NULL;
    }
  }
  printf("%d/%d passed\n", passed, datalen);

  if (p > 0) {
    printf("Some passes:\n");
    for (i = 0; i < p; i++) {
      puts(passes[i]);
    }
  }
  if (f > 0) {
    printf("Some fails:\n");
    for (i = 0; i < f; i++) {
      puts(fails[i]);
    }
  }

  for (i = 0; i < mwc_len; i++) {
    if (!mwc_eq(in[i], in_orig[i])) {
      E("(in[%d] = {%d,%d}) != (in_orig[%d] = {%d,%d})", i, in[i].w, in[i].z, i, in_orig[i], in_orig[i].w, in_orig[i].z);
      break;
    }
  }

  if (passed == datalen) {
    return 0;
  }
  return -1;
}

