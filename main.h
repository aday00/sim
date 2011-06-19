/*
 * main.h  Copyright 2011 Andrew Schaumberg
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
#define gfree   GC_free



/* A checked gmalloc.  WARNING: caller returns -1 on failure. */
#define allocreturn(buf, size) do { \
  buf = gmalloc(size); \
  if (!buf) { \
    E("Could not allocate %d bytes for variable \"" #buf "\"", size); \
    return -1; \
  } \
} while (0);
#define zallocreturn(buf, size) do { \
  allocreturn(buf, size); \
  memset(buf, 0, size); \
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
