/*
 * sum.cl  Copyright 2011 Andrew Schaumberg
 */

__kernel
void sum(__global const int *in, __global int *out, const unsigned int len) {
  int i, t;

  i = get_global_id(0); /* Current elm idx to process */
  if (i < len) {
    t = in[i];
    out[i] = (t * (t+1)) / 2;
  }
}
