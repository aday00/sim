/*
 * sum.cl  Copyright 2011 Andrew Schaumberg
 */

__kernel
void sum(__global const int *in, __global int *out) {
  /* Get the index of the current element to be processed */
  int i, t;
  i = get_global_id(0);
  t = in[i];
  out[i] = (i * (i+1)) / 2;
}
