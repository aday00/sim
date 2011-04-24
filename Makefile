CC=gcc
WD=$(shell pwd)

GC_SRC=$(WD)/gc-7.1
GC_LIB=$(WD)/lib/gc

all: main

gc:
	mkdir -p $(GC_LIB); \
	cd gc-7.1; \
	./configure --prefix=$(GC_LIB); \
	make; \
	make check; \
	make install

main: main.c sum.c
	gcc -Ilib/gc/include -Llib/gc/lib -lgc -lm main.c sum.c -o main
