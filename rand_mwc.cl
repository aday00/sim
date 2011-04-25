/*
 * rand.cl  Copyright 2011 Andrew Schaumberg
 *
 * Multiply-with-carry pRNG in the method of George Marsaglia.
 * This subroutine adapted from:
 * https://secure.wikimedia.org/wikipedia/en/wiki/Random_number_generation#Computational_methods
 */

#include "rand_mwc.h"

__kernel
void rand_mwc(__global const mwc_t *in, __global int *out) {
  mwc_t m;
  int i, o, t;

  i = get_global_id(0); /* Current elm idx to process */

  /* FIXME: maintain mw and mz in shared memory, generate many rands */
  m = in[i];
  
  /* Just to make the code harder to understand, redefine i to index the 'out'
   * array, which is twice as long as the 'in' array.
   * This reduces register usage and may boost speed.
   */
  i *= 2;

  m.w = 18000 * (m.w & 0xffff) + (m.w >> 16);
  m.z = 36969 * (m.z & 0xffff) + (m.z >> 16);
  t  = (m.z << 16) + m.w; /* first generated random */

  out[i] = t; /* first generated random */

  m.w = 18000 * (m.w & 0xffff) + (m.w >> 16);
  m.z = 36969 * (m.z & 0xffff) + (m.z >> 16);
  t  = (m.z << 16) + m.w; /* first generated random */

  out[i+1] = t; /* second generated random */
}
