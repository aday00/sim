/*
 * rand.cl  Copyright 2011 Andrew Schaumberg
 *
 * Multiply-with-carry pRNG in the method of George Marsaglia.
 * This subroutine adapted from:
 * https://secure.wikimedia.org/wikipedia/en/wiki/Random_number_generation#Computational_methods
 */

#include "rand_mwc.h"

__kernel
void rand_mwc(__global mwc_t *in, __global int *out) {
  int i;
  int o;
  int t;
  mwc_t m;
  __local mwc_t m_orig;

  i = get_global_id(0); /* Current elm idx to process */

  /* FIXME: maintain mw and mz in shared memory, generate many rands */
  /*m_orig =*/ m = in[i];
  
  /* Just to make the code harder to understand, redefine i to index the 'out'
   * array, which is twice as long as the 'in' array.
   * This reduces register usage and may boost speed.
   */
  //i *= 2;
  o = i * 2;

//  barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
  mwc_next(m);
//  barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
  t  = mwc_rand(m);
//  barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

  out[o] = t; /* first generated random */
//  barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

  mwc_next(m);
//  barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
  t  = mwc_rand(m);
//  barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);

  out[o+1] = t; /* second generated random */

  /* Use i to index the input array.  Write the updated mwc to 'in'. */
//  in[i] = m;
//  in[i] = (mwc_t) {0,0};
//  barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
//   m_orig = m;
//  m_orig.w = 1;
//  barrier(CLK_GLOBAL_MEM_FENCE | CLK_LOCAL_MEM_FENCE);
  // in[i] = m_orig;
}
