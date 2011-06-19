/*
 * Simplified rand_mwc.h for testing.
 *
 * swapmul.h  Copyright 2011 Andrew Schaumberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
