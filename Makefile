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
GEN_CFLAGS=$(GC_CFLAGS) $(CL_CFLAGS)

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

main: main.c sum.c
	$(CC) $(GEN_CFLAGS) -c sum.c -o sum.o
	$(CC) $(GEN_CFLAGS) -lm main.c sum.o -o main


