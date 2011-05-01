/*
 * swapmul.c  Copyright 2011 Andrew Schaumberg
 *
 * A toy mulwiply-with-carry-like test.
 */

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cl_job.h"
#include "cpu_util.h"
#include "gc.h"
#include "main.h"
#include "swapmul.h"

#define dump_cpu(i, o, l) do { \
  printf("BEGIN DUMP CPU\n"); \
  dump(i, o, l); \
} while(0);
#define dump_gpu(i, o, l) do { \
  printf("BEGIN DUMP GPU\n"); \
  dump(i, o, l); \
} while(0);
static void dump(sm_t *in, int *out, int sm_len)
{
  int i, j;
  for (i = 0; i < sm_len; i++) {
    j = i * 2;
    printf("j=%4d in[%3d].w=%8d out=%11d\n", j, i, in[i].w, out[j]);
    j++;
    printf("j=%4d in[%3d].z=%8d out=%11d\n", j, i, in[i].z, out[j]);
  }
}

int swapmul_gpu(int datalen, int iterations)
{
  unsigned int u, v, num_generators, rands_per_generator;

  int ret, i, j, t, p, f, sm_len, sm_bytes, passed,
      databytes, showlen, showbytes;
  int *out, *cpuout;
  char **passes, **fails;
  char *buf, **buf_to;
  size_t bufsize = 512;
  sm_t *in, *in_orig, m;

  cljob_ticket job;
  clbuf_ticket bufin, bufout;

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

  off_t sourcelen;
  char *source;

  int gpu = 1; /* use gpu opencl, not cpu opencl */
  char *platform_buf;
  int platform_bufsize = 128;
  char *progerr_buf;
  size_t progerr_bufsize = 2048, progerr_buflen;

  rands_per_generator = 2; /* Each pRNG creates this many randoms */
  num_generators = datalen / rands_per_generator;
  sm_len = num_generators;
  sm_bytes = sizeof(sm_t) * sm_len;

  ret = -1;
  databytes = sizeof(int) * datalen;

  /***
   * Allocate buffers, read OpenCL program source, setup input.
   */

  /* Show this many results each test */
  showlen   = (int)log((double)datalen);
  //showbytes = sizeof(int) * showlen;
  showbytes = sizeof(void *) * showlen;

  allocreturn(in,     sm_bytes);
  allocreturn(in_orig,sm_bytes);
  allocreturn(out,    databytes);
  allocreturn(cpuout, databytes);
  allocreturn(passes, showbytes);
  allocreturn(fails,  showbytes);
  allocreturn(platform_buf, platform_bufsize);

  callreturn( clregister(job) );
  callreturn( clbuild(job, "swapmul.cl", "swapmul") );

  /* Input for GPU, input cannot be 0 */
  for (u = 0; u <= sm_len; u++ ) {
    v = u * rands_per_generator;
    in[u] = (sm_t) { v+1, v+2 };
  }
  dump_gpu(in, out, sm_len);


  callreturn( clkargbuf(job, CL_MEM_READ_WRITE, sm_bytes,  in,
                        CL_TRUE, bufin) );
  callreturn( clkargbuf(job, CL_MEM_WRITE_ONLY, databytes, out,
                        CL_TRUE, bufout) );
  callreturn( clkargscalar(job, sizeof(int), &sm_len) );

  callreturn( clkargbufw(bufin) );

  /***
   * Run GPU kernel, read output, then release resources.
   */

  callreturn( clmemory(job, sm_bytes) );

  printf("i %d iter %d\n", i, iterations);
  for (i = 0; i < iterations; i++) {
    callreturn( clkernel(job) );
    /* Read results */
    callreturn( clkargbufr(bufout) );
    printf("\niteration %d/%d\n", i+1, iterations);
    dump_gpu(in, out, sm_len);
  }

  callreturn( clunregister(job) );
  callreturn( clunbuf(bufin)    );
  callreturn( clunbuf(bufout)   );

  /***
   * Validate results, print report
   */

#if 0
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

  if (passed == datalen) {
    return 0;
  }
  return -1;
#endif
  return 0;
}
int swapmul_cpu(int datalen, int iterations)
{
  unsigned int u, v, num_generators, rands_per_generator;
  int i, sm_len, sm_bytes, databytes;
  int *out;
  sm_t *in, m;

  rands_per_generator = 2; /* Each pRNG creates this many randoms */
  num_generators = datalen / rands_per_generator;
  sm_len = num_generators;
  sm_bytes = sizeof(sm_t) * sm_len;
  databytes = sizeof(int) * datalen;

  /***
   * Allocate buffers, read OpenCL program source, setup input.
   */

  allocreturn(in,     sm_bytes);
  allocreturn(out,    databytes);

  /* Input for GPU, input cannot be 0 */
  for (u = 0; u <= sm_len; u++ ) {
    v = u * rands_per_generator;
    in[u] = (sm_t) { v+1, v+2 };
  }
  dump_cpu(in, out, sm_len);

  /***
   * Run GPU kernel, read output, then release resources.
   */

  for (i = 0; i < iterations; i++) {
    printf("\niteration %d/%d\n", i+1, iterations);
    /* Do GPU work on CPU to check */
    for (u = 0; u <= sm_len; u++ ) {
      v = u * rands_per_generator;
      iterate(in, u, out, v, m);
    }
    dump_cpu(in, out, sm_len);
  }

  return 0;
}

int swapmul(int datalen, int iterations, int use_cpu)
{
  if (use_cpu) {
    return swapmul_cpu(datalen, iterations);
  } else {
    return swapmul_gpu(datalen, iterations);
  }
}

