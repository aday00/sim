/*
 * main.c  Copyright 2011 Andrew Schaumberg
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "cl_job.h"
#include "main.h"

void usage(void) {
  printf("usage\n");
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
  printf("hello, datalen is %d\n", datalen);
/*  sum(datalen); */
/*  rand_mwc(datalen); */
  swapmul(datalen, iterations, use_cpu);
  printf("hm\n");
  return 0;
}
