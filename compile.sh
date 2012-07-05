#!/bin/sh
gcc -O3 -g -std=gnu99 -o makebloom makebloom.c common.c
gcc -O3 -g -std=gnu99 -o bloomgrep bloomgrep.c common.c
