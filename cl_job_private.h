/*
 * OpenCL interface definitions for internal use.
 *
 * cl_job_private.h  Copyright 2011 Andrew Schaumberg
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
