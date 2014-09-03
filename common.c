#include "common.h"

#define NUMBER_OF_ROUNDS 4

// djb2 string hash function
unsigned long
hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void update_bloom(bloom_filter filter, unsigned char *str) {
  //printf("updating with '%s'\n", str);
  assert(*str != 0);
  for (int i=0; i<NUMBER_OF_ROUNDS; i++) {
    unsigned int str_hash = hash(str) & 0xfff;
    int filter_index = str_hash / 8;
    int bit_index = str_hash % 8;
    //printf("bumping bit in %i\n", filter_index);
    
    filter[filter_index] |= (1 << bit_index);
    
    (*str)++;
  }
  (*str) -= NUMBER_OF_ROUNDS;
}

// 1 for "might be a hit", 0 for "definitely unknown"
int check_bloom(bloom_filter filter, unsigned char *str) {
  assert(*str != 0);
  for (int i=0; i<NUMBER_OF_ROUNDS; i++) {
    unsigned int str_hash = hash(str) & 0xfff;
    int filter_index = str_hash / 8;
    int bit_index = str_hash % 8;
    
    if ((filter[filter_index] & (1 << bit_index)) == 0) {
      (*str) -= i;
      return 0;
    }
    
    (*str)++;
  }
  (*str) -= NUMBER_OF_ROUNDS;
  return 1;
}

void make_bloom_from_file(FILE *f, unsigned char *filter) {
  rewind(f);
  unsigned char buf[1024];
  int buf_i = 0;
  int c;
  while ((c = fgetc(f)) != EOF) {
    if (isalpha(c) || c == '_') {
      if (sizeof(buf) != buf_i) {
        buf[buf_i++] = c;
      }
    } else {
      if (sizeof(buf) != buf_i && buf_i > 0) {
        buf[buf_i] = 0;
        update_bloom(*(bloom_filter *)filter, buf);
      }
      buf_i = 0;
    }
  }
  if (ferror(f)) {
    puts("error occured while reading a file - exiting.");
    exit(1);
  }
}

bool is_text_file(FILE *f) {
  rewind(f);
  char head[1000];
  size_t len = fread(head, 1, sizeof(head), f);
  if (ferror(f)) {
    puts("error occured while reading a file - exiting.");
    exit(1);
  }
  if (len == 0) return false;
  int clean_chars = 0;
  for (int i = 0; i < len; i++) {
    char c = head[i];
    if ((c >= 0x20 && c <= 0x7E) || (c == '\t' || c == '\r' || c == '\n')) clean_chars++;
  }
  return clean_chars * 4 / len >= 3; // at least 3/4 should look like clean ASCII
}
