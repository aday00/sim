/*
 * Multiply-with-carry pRNG in the method of George Marsaglia.
 * This subroutine adapted from:
 * https://secure.wikimedia.org/wikipedia/en/wiki/Random_number_generation#Computational_methods
 *
 * swapmul.cl  Copyright 2011 Andrew Schaumberg
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
