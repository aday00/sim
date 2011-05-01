# Top-level makefile.  Copyright 2011 Andrew Schaumberg
	
CC=gcc
WD=$(shell pwd)

GC_SRC=$(WD)/gc-7.1
GC_LIB=$(WD)/lib/gc

CL_INC=/usr/include/nvidia-current/CL
CL_LIB=/usr/lib/nvidia-current

# C flags to build non-third-party programs
GC_CFLAGS=-Ilib/gc/include -Llib/gc/lib -lgc
CL_CFLAGS=-I$(CL_INC) -L$(CL_LIB) -lOpenCL
GDB_CFLAGS=-ggdb
GEN_CFLAGS=$(GC_CFLAGS) $(CL_CFLAGS) $(GDB_CFLAGS)

all: main

gc:
	mkdir -p $(GC_LIB); \
	cd gc-7.1; \
	./configure --prefix=$(GC_LIB); \
	make; \
	make check; \
	make install

clean:
	rm *.o main

mrproper: clean
	rm -rf $(GC_LIB)


cl_job.o: util.c
	$(CC) $(GEN_CFLAGS) -c cl_job.c -o cl_job.o

rand_mwc.o: rand_mwc.c
	$(CC) $(GEN_CFLAGS) -c rand_mwc.c -o rand_mwc.o

sum.o: sum.c
	$(CC) $(GEN_CFLAGS) -c sum.c -o sum.o

swapmul.o: swapmul.c
	$(CC) $(GEN_CFLAGS) -c swapmul.c -o swapmul.o

util.o: util.c
	$(CC) -c util.c -o util.o


main: main.c cl_job.o rand_mwc.o sum.o swapmul.o util.o
	$(CC) $(GEN_CFLAGS) -lm main.c cl_job.o rand_mwc.o sum.o swapmul.o util.o -o main


