/*
 * cl_job.c  Copyright 2011 Andrew Schaumberg
 *
 * Simple interface to setup, execute, and teardown an OpenCL job.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "cl_job.h"
#include "gc.h"
#include "main.h"
#include "queue.h"

inline _cljob_t *cljob_from_ticket(cljob_ticket t)
{
  return (_cljob_t *) t; /* Trivially convert a ticket to a job */
}

/* All jobs maintained in this singly-linked list */
typedef SLIST_HEAD(jobhead_s, _cljob_t) jobhead_t;
static jobhead_t *jobhead;
/* Elements of this singly-linked list are of this type */
typedef struct jobentry_s {
  cljob_ticket job;
  SLIST_ENTRY(_cljob_t) entry; /* entry refers to next jobentry in list */
} jobentry_t;

/* Call this once at program startup */
int clinit(void)
{
  allocreturn(jobhead, sizeof(jobhead_t));
  SLIST_INIT(jobhead);
}

/* Initialize state for this new job and push to job list */
int clregister(cljob_ticket job)
{
  jobentry_t *je;
  _cljob_t *jp;

  jp = cljob_from_ticket(job);
  allocreturn(job, sizeof(_cljob_t));
  memset(job, 0,   sizeof(_cljob_t));

  allocreturn(je,  sizeof(jobentry_t));
  je->job = job;

  SLIST_INSERT_HEAD(jobhead, je, entry);
  return 0;
}

/* Build source from file named sourcefn, returning sourcelen and source.
 */
int clbuild(cljob_ticket job, const char *sourcefn, off_t *sourcelen, char **source)
{
  /* For reading OpenCL program source */
  FILE *fp;
  struct stat info;
  char *src;
  off_t srclen;
  size_t srcread;
  int ret;

  ret = stat(sourcefn, &info);
  if (ret) {
    E("Could not get status of file %s, ret %d, err %d", sourcefn, ret, errno);
    return -1;
  }
  srclen = info.st_size; /* filesize of source */
  if (srclen <= 0) {
    E("File %s length of %ld invalid!", sourcefn, srclen);
    return -1;
  }

  fp = fopen(sourcefn, "r");
  if (!fp) {
    E("Failed to read program source file %s, err %d", sourcefn, errno);
    return -1;
  }
  alloclongreturn(src, srclen);
  srcread = fread(src, 1, srclen, fp);
  if (srcread < srclen) {
    E("Program source read of %s returned %d bytes!  Source is %ld bytes.",
      sourcefn, srcread, srclen);
    return -1; /* Cannot expect to continue */
  }
  I("Source in %s follows:\n%s", sourcefn, src);

  *sourcelen = srclen;
  *source    = src;

  return 0;
}
