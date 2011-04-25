/*
 * rand_mwc.h  Copyright 2011 Andrew Schaumberg
 *
 * Defines custom types for pRNG.
 */

#ifndef _RAND_MWC_H_
#define _RAND_MWC_H_

typedef struct mwc_s {
  unsigned int w; /* the low word */
  unsigned int z; /* the high word */
} mwc_t;

#define mwc_eq(a, b) (a.w == b.w && a.z == b.z) /* equality */

/* Advance state of mwc prng */
#define mwc_next(m) do { \
  m.w = 18000 * (m.w & 0xffff) + (m.w >> 16); \
  m.z = 36969 * (m.z & 0xffff) + (m.z >> 16); \
} while(0);

/* Get the random number at current state */
#define mwc_rand(m) ((m.z << 16) + m.w)

#endif /* _RAND_MWC_H_ */
