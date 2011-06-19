/*
 * Utility definitions, for either CPU or GPU.
 *
 * util.h  Copyright 2011 Andrew Schaumberg
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

#ifndef _UTIL_H_
#define _UTIL_H_

/* Define OpenCL keywords for CPU to ignore.
 * To allow OpenCL code to compile like any other C.
 */
#ifndef __SIM_USE_GPU__
#define __kernel
#define __global
#define __local

/* TBD: don't use a global, name-mangling may help */
extern int fake_global_id;
inline int get_global_id(int unused) {
  return fake_global_id++;
}
#endif /* !__SIM_USE_GPU__ */

/* Define OpenCL constants for CPU to ignore */
#ifndef __SIM_USE_GPU__
#define CLK_GLOBAL_MEM_FENCE ()
#endif /* !__SIM_USE_GPU__ */

/* Define wrappers that either GPU or CPU may use.
 * Generally, the CPU code compiles to no-op.
 */
#ifdef __SIM_USE_GPU__
#define gpubarrier(flags)       barrier(flags)
#else /* !__SIM_USE_GPU__ */
#define gpubarrier(flags)       do {} while (0);
#endif /* __SIM_USE_GPU__ */

#endif /* _UTIL_H_ */
