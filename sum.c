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
  cl_platform_id   platid;
  cl_platform_id*  platids;
  cl_context       ctx;
  cl_command_queue cmdq;
  cl_program       prog;
  cl_kernel        kern;

  cl_uint          num_platforms;

  cl_mem gpuin, gpuout; /* device memory for input and output arys */
  int gpu = 1; /* use gpu */
  char *platform_buf;


  ret = -1;
  databytes = sizeof(int) * datalen;

  /***
   * Allocate buffers
   */

  /* Show this many results each test */
  showlen   = (int)log((double)datalen);
  showbytes = sizeof(int) * showlen;

  allocreturn(in,     databytes);
  allocreturn(out,    databytes);
  allocreturn(gpuout, showbytes);
  allocreturn(passes, showbytes);
  allocreturn(fails,  showbytes);
  allocreturn(platform_buf, 128);

  /***
   * Setup GPU
   */
  devtype = gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU;

  /* Get platform before getdeviceids
   * http://developer.amd.com/Support/KnowledgeBase/Lists/KnowledgeBase/DispForm.aspx?ID=71
   */
  ret = clGetPlatformIDs(0, NULL, &num_platforms);
  if (ret) {
    E("clGetPlatformIDs ret %d, num platforms %d.", ret, num_platforms);
  }

  if (num_platforms > 0) {
    allocreturn(platids, sizeof(cl_platform_id) * num_platforms);
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

