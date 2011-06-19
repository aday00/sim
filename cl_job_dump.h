/*
 * Simple interface to setup, execute, and teardown an OpenCL job.
 *
 * cl_job.h  Copyright 2011 Andrew Schaumberg
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

#ifndef __CL_JOB_DUMP_H__
#define __CL_JOB_DUMP_H__

#include "main.h"

typedef struct cldump_s {
  cl_device_id     devid;
  cl_device_type   devtype;
  cl_platform_id   platform;
  cl_uint          platformslen;
  cl_platform_id  *platforms;
  cl_context       context;
  cl_command_queue cmdq;
  cl_context       context;
  cl_command_queue cmdq;
  cl_program       prog;
  cl_kernel        kern;
  cl_uint          kargc; /* count of kernel arguments */
  void           **kargv;
  cl_unit          memslen;
  cl_mem          *mems;
} cldump_t;

extern int cldump(cldump_t &dump);

#endif /* __CL_JOB_DUMP_H__ */
