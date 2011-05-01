/*
 * cl_job.h  Copyright 2011 Andrew Schaumberg
 *
 * Simple interface to setup, execute, and teardown an OpenCL job.
 */

#ifndef __CL_JOB_H__
#define __CL_JOB_H__

#include "main.h"

typedef struct _cljob_s {
  /* XXX: maintain a linked list of buffers for freeing at unreg-time */

  size_t global, local; /* memory sizes for gpu calculations */

  cl_command_queue cmdq;
  cl_program       prog;
  cl_kernel        kern;
} _cljob_t;

/* This is how Users refer to their job, an "opaque" ticket */
typedef _cljob_t *cljob_ticket; /* user interface */

typedef struct _clbuf_s {
  cl_mem        devmem;    /* gpu memory */
  size_t        hostbytes; /* host many bytes hostmem is in total */
  void         *hostmem;   /* cpu memory */
  cl_bool       blocking;  /* blocking read/write */
  cljob_ticket  job;       /* job that this clbuf belongs to */
} _clbuf_t;

/* This is how Users refer to an IO buffer for their app */
typedef _clbuf_t *clbuf_ticket; /* user interface */


extern int clinit(void);

extern int clregister(cljob_ticket job);
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
extern int clunbuf(clbuf_ticket buf);

#endif /* __CL_JOB_H__ */
