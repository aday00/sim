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
  int ret, i, j, sm_len, sm_bytes, databytes;
  int *out;
  sm_t *in;
  cljob_ticket job;
  clbuf_ticket bufin, bufout;

  rands_per_generator = 2; /* Each pRNG creates this many randoms */
  num_generators = datalen / rands_per_generator;
  sm_len = num_generators;
  sm_bytes = sizeof(sm_t) * sm_len;
  databytes = sizeof(int) * datalen;

  /* Create job, building program and alloating buffers. */
  callreturn( clregister(&job) );
  callreturn( clbuild(job, "swapmul.cl", "swapmul") );

  /* Input and output buffers for GPU */
  allocreturn(in,  sm_bytes);
  allocreturn(out, databytes);
  for (u = 0; u <= sm_len; u++ ) {
    v = u * rands_per_generator;
    in[u] = (sm_t) { v+1, v+2 };
  }
  dump_gpu(in, out, sm_len);

  /* Set up kernel arguments, and memory layout. */
  callreturn( clkargbuf(job, CL_MEM_READ_WRITE, sm_bytes,  in,
                        CL_TRUE, bufin) );
  callreturn( clkargbuf(job, CL_MEM_WRITE_ONLY, databytes, out,
                        CL_TRUE, bufout) );
  callreturn( clkargscalar(job, sizeof(int), &sm_len) );
  callreturn( clmemory(job, sm_bytes) );

  /* Write memory, run GPU kernel, read output, then release resources. */
  callreturn( clkargbufw(bufin) );
  printf("i %d iter %d\n", i, iterations);
  for (i = 0; i < iterations; i++) {
    callreturn( clkernel(job) );
    /* Read results */
    callreturn( clkargbufr(bufout) );
    printf("\niteration %d/%d\n", i+1, iterations);
    dump_gpu(in, out, sm_len);
  }
  callreturn( clunregister(job) );
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

