/*
 * main.c  Copyright 2011 Andrew Schaumberg
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "main.h"

void usage(void) {
  printf("usage\n");
}

int main(int argc, char** argv)
{
  int datalen = 0;
  char *s = NULL;
  int o;

  while ((o = getopt(argc, argv, "hl:")) != -1) {
    switch (o) {
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
  printf("hello, datalen is %d\n", datalen);
  sum(datalen);
  rand_mwc(datalen);
  return 0;
}
