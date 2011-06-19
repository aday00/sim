/*
 * main.c  Copyright 2011 Andrew Schaumberg
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cl_job.h"
#include "main.h"

void usage(void) {
  printf("usage: main [-c] -i iterations -l datalength\n"
         "-c: use CPU gather than GPU\n"
         "-i iterations: re-run algorithm on outputs iterations-1 many times\n"
         "-l datalength: number of rows/entries to process\n"
        );
}

int main(int argc, char** argv)
{
  int datalen = 0;
  int use_cpu = 0; /* by default, use GPU for testing */
  int iterations = 1;
  char *s = NULL;
  int o;

  while ((o = getopt(argc, argv, "ci:l:h")) != -1) {
    switch (o) {
    case 'c':
      use_cpu = 1;
      break;
    case 'i':
      iterations = (int)strtoul(optarg, NULL, 0);
      break;
    case 'l':
      datalen = (int)strtoul(optarg, NULL, 0);
      break;
    case 'h':
    default:
      usage();
      exit(1);
      break;
    }
  }

  ginit();
  clinit();
//  clexit();
/*  sum(datalen); */
/*  rand_mwc(datalen); */
  swapmul(datalen, iterations, use_cpu);
  return 0;
}
