#!/bin/bash
g++ -Iinclude -Wall -O3 -g -c -o $1.o $1.cc
g++ -Wall -O3 -g $1.o -o $1 -Llib -lrgbmatrix -lrt -lm -lpthread -lpng -lz
