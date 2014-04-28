#include "common.h"
#include <stdbool.h>

static char **folder_blacklist;

static bool folder_bad(char *name) {
  for (char **badp = folder_blacklist; *badp != NULL; badp++)
    if (!strcmp(*badp, name)) return true;
  return false;
}

static void bloom_add_folder(path *path, FILE *bloom_file) {
  char *oldnull = *path + strlen(*path);
  DIR *d = opendir(*path);
  if (d == NULL) {
    puts("warning: opendir failed");
    return;
  }
  
  struct dirent e;
  struct dirent *e2;
  while (1) {
    if (readdir_r(d, &e, &e2) != 0) {
      puts("warning: readdir failed");
      closedir(d);
      return;
    }
    if (e2 == NULL) {
      closedir(d);
      return;
    }
    if (strlen(*path) + strlen(e.d_name) + 1 + 1 > sizeof(*path)) {
      puts("warning: blocked traversal - path too long");
      closedir(d);
      return;
    }
    if (!strcmp(e.d_name, ".") || !strcmp(e.d_name, "..") || !strcmp(e.d_name, ".wordgrep_bloom.db")) {
      continue;
    }
    strcat(*path, e.d_name);
    FILE *f;
    switch (e.d_type) {
      case DT_UNKNOWN:
        puts("warning: dirent with unknown type, too lazy to stat it");
        break;
      case DT_DIR:
        if (!folder_bad(e.d_name)) {
          strcat(*path, "/");
          bloom_add_folder(path, bloom_file);
        }
        break;
      case DT_REG:
        f = fopen(*path, "r");
        if (f == NULL) {
          puts("warning: fopen() failed");
          break;
        }
        assert(fwrite(*path, strlen(*path)+1, 1, bloom_file) == 1);
        printf("blooming %s\n", *path);
        bloom_filter filter;
        memset(&filter, 0, sizeof(filter));
        make_bloom_from_file(f, filter);
        fclose(f);
        assert(fwrite(&filter, sizeof(bloom_filter), 1, bloom_file) == 1);
        break;
      default:
        break;
    }
    *oldnull = 0;
  }
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    puts("wrong invocation, please use `makebloom folder [badfolder1 [badfolder2 [...]]`");
    return 1;
  }
  char *folder = argv[1];
  folder_blacklist = argv+2;
  
  char *oldcwd = get_current_dir_name();
  if (chdir(folder) != 0) {
    puts("can't change folder. aborting.");
    return 1;
  }
  
  FILE *bloom_file = fopen(".wordgrep_bloom.db", "w");
  if (bloom_file == NULL) {
    printf("can't open bloom file for writing: %s. aborting.\n", strerror(errno));
    return 1;
  }
  
  path p;
  strcpy(p, "./");
  bloom_add_folder(&p, bloom_file);
  
  chdir(oldcwd);
  free(oldcwd);
  return 0;
}
