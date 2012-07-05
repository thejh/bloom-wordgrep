#include "common.h"

int main(int argc, char *argv[]) {
  if (argc < 3) {
    puts("wrong invocation, please use `makebloom folder word1[,word2[,...]]`");
    return 1;
  }
  char *folder = argv[1];
  
  char *oldcwd = get_current_dir_name();
  if (chdir(folder) != 0) {
    puts("can't change folder. aborting.");
    return 1;
  }
  
  FILE *bloom_file = fopen(".wordgrep_bloom.db", "r");
  if (bloom_file == NULL) {
    printf("can't open bloom file for reading: %s. aborting.\n", strerror(errno));
    return 1;
  }
  
  // prepare the bloom pattern we're looking for
  bloom_filter filter;
  memset(&filter, 0, sizeof(filter));
  for (int i=2; i<argc; i++) {
    update_bloom(filter, argv[i]);
  }
  
  int file_count = 0, match_count = 0;

  while (1) {
    //printf("reading filename at %li\n", ftell(bloom_file));
    //if (fscanf(bloom_file, "%as", &path) <= 0) break;
    path p;
    int p_index = 0;
    int c;
    do {
      c = fgetc(bloom_file);
      p[p_index++] = c;
    } while (c > 0 && c < 256);
    if (c != 0) goto end;
    //printf("reading filter at %li\n", ftell(bloom_file));
    bloom_filter read_filter;
    assert(sizeof(bloom_filter) == 512);
    assert(fread(read_filter, sizeof(bloom_filter), 1, bloom_file) == 1);
    for (int j=0; j<512; j++) {
      if (filter[j] != 0) {
        //printf("need %x, found %x, AND is %x at pos %i\n", filter[j], read_filter[j], filter[j] & read_filter[j], j);
      }
      if ((filter[j] & read_filter[j]) != filter[j]) {
        goto continue_fileloop;
      }
    }
    printf("possible match in %s\n", p);
    match_count++;
continue_fileloop:
    file_count++;
  }
  
end:
  chdir(oldcwd);
  free(oldcwd);
  
  printf("stats: %i files searched, %i hits (%f%%)\n", file_count, match_count, 100 * match_count / (double) file_count);
  
  return 0;
}
