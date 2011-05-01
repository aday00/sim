/*
 * swapmul.h  Copyright 2011 Andrew Schaumberg
 *
 * Simplified rand_mwc.h for testing.
 */

#ifndef _RAND_MWC_H_
#define _RAND_MWC_H_

typedef struct sm_s {
  unsigned int w; /* the low word */
  unsigned int z; /* the high word */
} sm_t __attribute__ ((aligned(8)));

#if 0 /* fake */
#define iterate(in, in_idx, out, out_idx, m) do { \
  m = in[in_idx]; /* read */ \
  out[out_idx]     = m.w; \
  out[(out_idx)+1] = m.z; \
} while(0);
#else
#define iterate(in, in_idx, out, out_idx, m) do { \
  m = in[in_idx]; /* read */ \
  m = (sm_t) { m.z + (in_idx*100), m.w }; /* swapmul */ \
  in[(in_idx)] = m; /* update state in "in" */ \
  out[out_idx]     = m.w; \
  out[(out_idx)+1] = m.z; \
} while(0);
#endif /* fake */

#endif /* _RAND_MWC_H_ */
