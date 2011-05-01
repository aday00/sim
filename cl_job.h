/*
 * cl_job.h  Copyright 2011 Andrew Schaumberg
 *
 * Simple interface to setup, execute, and teardown an OpenCL job.
 */

#ifndef __CL_JOB_H__
#define __CL_JOB_H__

#include "main.h"

typedef struct _cljob_s {
  size_t global, local; /* memory sizes for gpu calculations */

  cl_device_id     devid;
  cl_device_type   devtype;
  cl_platform_id   platform;
  cl_platform_id  *platforms;
  cl_uint          platformslen;
  cl_context       context;
  cl_command_queue cmdq;
  cl_program       prog;
  cl_kernel        kern;

  cl_mem          *memories;
} _cljob_t;

/* This is how Users refer to their job, an "opaque" ticket */
typedef _cljob_t *cljob_ticket;

#endif /* __CL_JOB_H__ */
