/*
 * cl_job_private.h  Copyright 2011 Andrew Schaumberg
 *
 * OpenCL interface definitions for internal use.
 */

#ifndef __CL_JOB_PRIVATE_H__
#define __CL_JOB_PRIVATE_H__

#include "main.h"
#include "queue.h"

/* Elements of the singly-linked list of clbuf are of this type */
typedef struct bufentry_s {
  struct _clbuf_s         *buf;
  SLIST_ENTRY(bufentry_s)  entry; /* entry points to next bufentry in list */
} bufentry_t;

/* All clbufs for one cljob maintained in this singly-linked list */
typedef SLIST_HEAD(bufhead_s, bufentry_s) bufhead_t;

typedef struct _cljob_s {
  size_t global, local; /* memory sizes for gpu calculations */
  cl_command_queue cmdq;
  cl_program       prog;
  cl_kernel        kern;

  bufhead_t       *bufhead; /* ptr to list of clbuf for freeing */
} _cljob_t;

typedef struct _clbuf_s {
  cl_mem        devmem;    /* gpu memory */
  size_t        hostbytes; /* host many bytes hostmem is in total */
  void         *hostmem;   /* cpu memory */
  cl_bool       blocking;  /* blocking read/write */

  _cljob_t     *jobptr;    /* job that this clbuf belongs to */
} _clbuf_t;

#endif /* __CL_JOB_PRIVATE_H__ */
