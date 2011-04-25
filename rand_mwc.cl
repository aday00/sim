/*
 * rand.cl  Copyright 2011 Andrew Schaumberg
 *
 * Multiply-with-carry pRNG in the method of George Marsaglia.
 * This subroutine adapted from:
 * https://secure.wikimedia.org/wikipedia/en/wiki/Random_number_generation#Computational_methods
 */

__kernel
void rand_mwc(__global const int *in, __global int *out, const unsigned int len) {
  unsigned int mw, mz; /* mz is the "high word", mw is the "low word" */
  int i, t;

  i = get_global_id(0); /* Current elm idx to process */

  /* FIXME: maintain mw and mz in shared memory, generate many rands */
  i  *= 2; /* Two rands generated per generator */
  mw = in[i];
  mz = in[i+1];

  mw = 18000 * (mw & 0xffff) + (mw >> 16);
  mz = 36969 * (mz & 0xffff) + (mz >> 16);
  t  = (mz << 16) + mw; /* first generated random */

  out[i  ] = t; /* first generated random */

  mw = 18000 * (mw & 0xffff) + (mw >> 16);
  mz = 36969 * (mz & 0xffff) + (mz >> 16);
  t  = (mz << 16) + mw; /* first generated random */

  out[i+1] = t; /* second generated random */
}
