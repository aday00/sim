/*
 * cl_job.h  Copyright 2011 Andrew Schaumberg
 *
 * Simple interface to setup, execute, and teardown an OpenCL job.
 */

#ifndef __CL_JOB_H__
#define __CL_JOB_H__

#include "main.h"
//#include "cl_job_private.h"

/* This is how Users refer to their job, an "opaque" ticket */
typedef struct _cljob_s *cljob_ticket; /* user interface */

/* This is how Users refer to an IO buffer for their app */
typedef struct _clbuf_s *clbuf_ticket; /* user interface */

extern int clinit(void);
extern int clregister(cljob_ticket *job); /* fills in job */
extern int clbuild(cljob_ticket job, const char *sourcefn,
                   const char *kernelfunc);

extern int clkargscalar(cljob_ticket job, size_t scalarbytes,
                        const void *scalarp);
extern int clkargbuf(   cljob_ticket job, cl_mem_flags flags, size_t hostbytes,
                        void *hostmem, cl_bool blocking, clbuf_ticket buf);
extern int clkargbufw(clbuf_ticket buf);
extern int clkargbufr(clbuf_ticket buf);

extern int clmemory(cljob_ticket job, size_t globalbytes);
extern int clkernel(cljob_ticket job);

extern int clunregister(cljob_ticket job);
extern int clexit(void);

#endif /* __CL_JOB_H__ */
