#define _GNU_SOURCE

#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

typedef unsigned char bloom_filter[512];
typedef char path[4096];

void make_bloom_from_file(FILE *f, unsigned char *filter);
void update_bloom(bloom_filter filter, unsigned char *str);
