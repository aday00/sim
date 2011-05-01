/*
 * util.h  Copyright 2011 Andrew Schaumberg
 *
 * Utility definitions, for either CPU or GPU.
 */

#ifndef _UTIL_H_
#define _UTIL_H_

/* Define OpenCL constants for CPU to ignore */
#ifndef __SIM_USE_GPU__
#define CLK_GLOBAL_MEM_FENCE (0)
#endif /* !__SIM_USE_GPU__ */

#ifdef __SIM_USE_GPU__
#define gpubarrier barrier
#else /* !__SIM_USE_GPU__ */
#define gpubarrier do {} while (0);
#endif /* __SIM_USE_GPU__ */

#endif /* _UTIL_H_ */
