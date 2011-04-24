/*
 * sum.c  Copyright 2011 Andrew Schaumberg
 *
 * Calculate sums of all integers preceding and including given integer.
 * inputs:  0  1  2  3  4  5 ...
 * outputs: 0  1  3  6 10 15 ...
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gc.h"
#include "main.h"

int sum (int datalen) {
  int ret, i, t, p, f, passed,
      databytes, showlen, showbytes;
  int *in, *out, *passes, *fails;

  size_t global, local; /* memory sizes for gpu calculations */

  cl_device_id     devid;
  cl_device_type   devtype;
  cl_platform_id   platform = NULL;
  cl_platform_id*  platforms;
  cl_context       context;
  cl_command_queue cmdq;
  cl_program       prog;
  cl_kernel        kern;

  cl_uint          platformslen;

  cl_mem gpuin, gpuout; /* device memory for input and output arys */
  int gpu = 1; /* use gpu */
  char *platform_buf;
  int platform_buflen = 128;

  /* For reading OpenCL program source */
  FILE *fp;
  char *source;
  char *sourcefn = "sum.cl";
  size_t sourcelen;


  ret = -1;
  databytes = sizeof(int) * datalen;

  /***
   * Allocate buffers, read OpenCL program source.
   */

  /* Show this many results each test */
  showlen   = (int)log((double)datalen);
  showbytes = sizeof(int) * showlen;

  allocreturn(in,     databytes);
  allocreturn(out,    databytes);
  allocreturn(passes, showbytes);
  allocreturn(fails,  showbytes);
  allocreturn(platform_buf, platform_buflen);

  fp = fopen(sourcefn, "r");
  if (!fp) {
    E("Failed to read kernel source");
    return -1;
  }
  source = gmalloc(MAX_CL_SOURCE_SIZE); /* get file size */
  sourcelen = fread(source, 1, MAX_CL_SOURCE_SIZE, fp);
  if (sourcelen <= 0) {
    E("Kernel source read of %s returned %d bytes!", sourcefn, sourcelen);
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
    ret = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, platform_buflen,
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

  cmdq = clCreateCommandQueue(context, devid, 0, &ret);
  if (cmdq) {
    if (ret != CL_SUCCESS) {
      I("Command queue creation returned %d", ret);
    }
  } else {
    E("Command queue creation failed %d!", ret);
    return -1;
  }



  for (i = 0; i < datalen; i++) {
    in[i] = i;
  }
  for (i = 0; i < datalen; i++) {
    out[i] = (i * (i + 1)) / 2;
  }


  printf("sum test: ");
  f = p = passed = 0;
  for (i = 0; i < datalen; i++) {
    t = in[i];
    if (out[i] == (t * (t+1))/2) {
      passed++;
      if (p < showlen) {
        passes[p++] = i;
      }
    } else {
      if (f < showlen) {
        fails[f++] = i;
      }
    }
  }
  printf("%d/%d passed\n", passed, datalen);

  if (p > 0) {
    printf("Some passes:\n");
    for (i = 0; i < p; i++) {
      t = passes[i];
      printf("in[%4d] = %10d\tout[%4d] = %10d\n",
             t, in[t], t, out[t]);
    }
  }
  if (f > 0) {
    printf("Some fails:\n");
    for (i = 0; i < f; i++) {
      t = fails[i];
      printf("in[%4d] = %10d\tout[%4d] = %10d\n",
             t, in[t], t, out[t]);
    }
  }

  if (passed == datalen) {
    return 0;
  }
  return -1;
}

