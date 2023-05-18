#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include "encode.h"
#include <fcntl.h>

typedef enum { false, true } bool;
extern char *optarg;

void directories_traversal(char *path);
void add_to_tarfile(char *to_add);
int isDirectory(char *path);

void usage() {
  fprintf(stderr, "usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  char opt;
  int i = 0;
  char *path;

  /* Option flags */
  char *output = NULL;
  bool create_archive = false, 
       print_contents = false, 
       extract_contents = false, 
       verbose = false, 
       strict = false;

  if (argc <= 3) {
    usage();
  }

  /* Parse options and update flags */
  while ((opt = argv[1][i++])) {
    switch (opt) {
      case 'f':
        output = argv[2];
        break;
      case 'c':
        create_archive = true;
        break;
      case 't':
        print_contents = true;
        break;
      case 'x':
        extract_contents = true;
        break;
      case 'v':
        verbose = true;
        break;
      case 'S':
        strict = true;
        break;
    }
  }
  printf("%s %d %d %d %d %d\n---------\n", output, create_archive, print_contents, extract_contents, verbose, strict);
  if (!output)
    usage();
  if (!create_archive && !print_contents && !extract_contents)
    usage();

  /* passing the path through the traversal function */
  path = argv[3];
  printf("this is the path: %s\n", path);
  directories_traversal(path);

  char *header = create_archive_header(argv[3]);
  for (i = 0; i < 512; i ++) {
    if (header[i])
      printf("%x ", header[i]);
    else
      printf("_");
  }
  printf("\n");
  int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
  write(fd_out, header, 512);

  return 0;
}

/* level-order DFS of the directory */
void directories_traversal(char *path) {

  DIR *dir;
  struct dirent *dir_read;
  struct stat stat_buffer;
  int i=0;
  int num_traversals = 0;
  char child_path[PATH_MAX][PATH_MAX];

  if (isDirectory(path) != 0) {
    /* opening the directory */
    dir = opendir(path);
    if (dir == NULL) {
      perror("opendir");
      exit(EXIT_FAILURE);
    }
  }
  else {
    /* base case for a file */
    char file_path[PATH_MAX];
    snprintf(file_path, PATH_MAX, "%s/%s", dirname(path), basename(path));
    printf("path: %s\n", path);
    add_to_tarfile(path);
    return;
  }

  /* iterating through all the directory's subdirectories or files */
  while((dir_read = readdir(dir)) != NULL) {
    
    /* ignoring self and parent */
    if (strcmp(dir_read->d_name, ".") == 0 || strcmp(dir_read->d_name, "..") == 0) {
      continue;
    }

    printf("curr name: %s\n", dir_read->d_name);
    /* statting the current child */
    snprintf(child_path[i], PATH_MAX, "%s/%s", path, dir_read->d_name);

    if (stat(child_path[i], &stat_buffer) == -1) {
      perror ("stat");
      exit(EXIT_FAILURE);
    }

    printf("path: %s\n", child_path[i]);
    add_to_tarfile(child_path[i]);
    i++;
    num_traversals++;
  }

  /* recursing for each child */
  for (i=0; i<num_traversals; i++) {
    directories_traversal(child_path[i]);
  }
}

void add_to_tarfile(char *to_add) {
  /* not yet implemented */
}

int isDirectory(char *path) {
  struct stat sb;
  if (stat(path, &sb) == -1) {
    perror("stat");
    exit(EXIT_FAILURE);
  }
  /* returns 0 if it's not a directory */
  return S_ISDIR(sb.st_mode);
}
