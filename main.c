/*
 * main.c  Copyright 2011 Andrew Schaumberg
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
