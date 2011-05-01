/*
 * swapmul.cl  Copyright 2011 Andrew Schaumberg
 *
 * Multiply-with-carry pRNG in the method of George Marsaglia.
 * This subroutine adapted from:
 * https://secure.wikimedia.org/wikipedia/en/wiki/Random_number_generation#Computational_methods
 */

#include "swapmul.h"
#include "gpu_util.h"

__kernel
void swapmul(__global sm_t *in, __global int *out, const int len) {
  int i, o;
  sm_t m;

  i = get_global_id(0); /* Current elm idx to process */
  if (i >= len) return; /* Discard unneccessary ranks if created */

  o = 2*i;
  iterate(in, i, out, o, m);
}
