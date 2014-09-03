#!/bin/sh
gcc -Wall -O3 -g -std=gnu99 -o makebloom makebloom.c common.c
gcc -Wall -O3 -g -std=gnu99 -o bloomgrep bloomgrep.c common.c
