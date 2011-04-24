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
  int ret, i, t, passed,
      databytes, showlen, showbytes;
  int *in, *out, *passes, *fails;

  ret = -1;
  databytes = sizeof(int) * datalen;

  in = gmalloc(databytes);
  if (!in) {
    E("Cannot allocate input buffer");
    return -1;
  }

  out = gmalloc(databytes);
  if (!out) {
    E("Cannot allocate output buffer");
    return -1;
  }

  /* Show this many results each test */
  showlen   = (int)log((double)datalen);
  showbytes = sizeof(int) * showlen;

  passes = gmalloc(showbytes);
  if (!passes) {
    E("Cannot allocate passes buffer\n");
    return -1;
  }

  fails = gmalloc(showbytes);
  if (!fails) {
    E("Cannot allocate fails buffer\n");
    return -1;
  }



  for (i = 0; i < datalen; i++) {
    in[i] = i;
  }
  for (i = 0; i < datalen; i++) {
    out[i] = (i * (i + 1)) / 2;
  }


  printf("sum test: ");
  passed = 0;
  for (i = 0; i < datalen; i++) {
    t = in[i];
    if (out[i] == (t * (t+1))/2) {
      passed++;
    }
  }
  printf("%d/%d passed\n", passed, datalen);
  if (passed == datalen) {
    return 0;
  }
  return -1;
}

