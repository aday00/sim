/*
 * main.h  Copyright 2011 Andrew Schaumberg
 */

#ifndef _MAIN_H_
#define _MAIN_H



/* These include paths/filenames differ by environment */
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif



#define PRINT_DEBUG 1
#if PRINT_DEBUG > 1
#define E( msg, ... ) printf( "%s: ERROR: %s, line %d: " msg "\n", \
		              __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#define I( msg, ... ) printf( "%s: INFO:  %s, line %d: " msg "\n", \
		              __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#elif PRINT_DEBUG == 1
#define E( msg, ... ) printf( "%s: ERROR: %s, line %d: " msg "\n", \
		              __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__ )
#define I( msg, ... ) printf( "%s: INFO: " msg "\n", \
		              __FILE__, ##__VA_ARGS__ )
#else
#define E( msg, ... ) do { } while (0)
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


#endif /* _MAIN_H_ */
