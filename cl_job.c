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

#include "gc.h"
#include "main.h"

typedef struct cljob_s {
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
} cljob_t;

static cljob_t jobs[];

/* Build source from file named sourcefn, returning sourcelen and source.
 */
int clbuild(const char *sourcefn, off_t *sourcelen, char **source)
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
