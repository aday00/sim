/*
 * rand_mwc.h  Copyright 2011 Andrew Schaumberg
 *
 * Defines custom types for pRNG.
 */

#ifndef _RAND_MWC_H_
#define _RAND_MWC_H_

typedef struct mwc_s {
  unsigned int mw;
  unsigned int mz;
} mwc_t;

#endif /* _RAND_MWC_H_ */
