/*
 * main.h  Copyright 2011 Andrew Schaumberg
 */

#ifndef _MAIN_H_
#define _MAIN_H

#include "gc.h" /* for garbage-collected memory */
#include "string.h" /* for memset */


/* These include paths/filenames differ by environment */
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif



#define PRINT_DEBUG 1
#define eprintf(...) fprintf( stderr, ##__VA_ARGS__ )
#if PRINT_DEBUG > 1
#define E( msg, ... ) eprintf( "%s: ERROR: %s, line %d: " msg "\n", \
                               __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#define T( msg, ... ) eprintf( "%s: TRACE: %s, line %d: " msg "\n", \
                               __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#define I( msg, ... ) eprintf( "%s: INFO:  %s, line %d: " msg "\n", \
                               __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#elif PRINT_DEBUG == 1
#define E( msg, ... ) eprintf( "%s: ERROR: %s, line %d: " msg "\n", \
                               __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#define T( msg, ... ) eprintf( "%s: TRACE: %s, line %d: " msg "\n", \
                               __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#define I( msg, ... ) eprintf( "%s: INFO: " msg "\n", \
                               __FILE__, ##__VA_ARGS__ )
#else
#define E( msg, ... ) do { } while (0)
#define T( msg, ... ) do { } while (0)
#define I( msg, ... ) do { } while (0)
#endif



/* Aliases for gc.h's functions */
#define ginit   GC_init
#define gmalloc GC_malloc



/* A checked gmalloc.  WARNING: caller returns -1 on failure. */
#define allocreturn(buf, size) do { \
  buf = gmalloc(size); \
  if (!buf) { \
    E("Could not allocate %d bytes for variable \"" #buf "\"", size); \
    return -1; \
  } \
} while (0);
#define zallocreturn(bug, size) do { \
  allocreturn(bug, size); \
  memset(clhw, 0, size); \
} while (0);

/* Another, for long sizes */
#define alloclongreturn(buf, longsize) do { \
  buf = gmalloc(longsize); \
  if (!buf) { \
    E("Could not allocate %ld bytes for variable \"" #buf "\"", longsize); \
    return -1; \
  } \
} while (0);



/* Safe quick garbage-collected caller-returning sprintf */
#define gprintfreturn(buf, bufsize, ...) do { \
  allocreturn(buf, bufsize); \
  snprintf(buf, bufsize, ##__VA_ARGS__); \
} while (0);


/* Similar pattern: call a function, complain if error, and return. */
#define callreturn(invoke) { \
  int __r; \
  do { \
    __r = (invoke); \
    if (__r) { \
      E("%s returned %d", #invoke, __r); \
      return __r; \
    } \
  } while (0); \
}


#endif /* _MAIN_H_ */
