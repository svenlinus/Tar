#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "encode.h"
#include "decode.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

extern char *optarg;

void usage() {
  fprintf(stderr, "usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  char opt;
  int i = 0;
  struct stat sb;

  /* Option flags */
  char *output = NULL;
  bool create_archive = false, 
       print_contents = false, 
       extract_contents = false, 
       verbose = false, 
       strict = false;

  if (argc <= 2)
    usage();

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
  if (create_archive + print_contents + extract_contents > 1) {
    fprintf(stderr, "You may only choose one of the 'ctx' options.\n");
    usage();
  }
  if (create_archive + print_contents + extract_contents < 1) {
    fprintf(stderr, "you must choose one of the 'ctx' options.\n");
    usage();
  }

  /* passing the path through the traversal function */
  // path = argv[3];
  // printf("this is the path: %s\n", path);
  // directories_traversal(path);

  if (print_contents) {
    int fd_in = open(argv[2], O_RDONLY);
    if (fd_in < 0) {
      perror("open");
      exit(EXIT_FAILURE);
    }
    list_contents(fd_in, verbose);
  }
  else if (create_archive) {
    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out < 0) {
      perror("open");
      exit(EXIT_FAILURE);
    }
    if (lstat(argv[3], &sb) < 0) {
      perror("lstat");
      exit(EXIT_FAILURE);
    }
    if (!(S_ISDIR(sb.st_mode))) {
      add_to_tarfile(argv[3], fd_out);
    }
    else {
      if (lstat(argv[3], &sb) < 0) {
        perror("lstat");
        exit(EXIT_FAILURE);
      }
      if (S_ISDIR(sb.st_mode)) {
        char temp[256];
        strcpy(temp, argv[3]);
        strcat(temp, "/");
        add_to_tarfile(temp, fd_out);
      }
      traverse_directory(argv[3], fd_out);
    }
    char *two_null_blocks = (char *)calloc(BLOCK_LEN * 2, 1);
    write(fd_out, two_null_blocks, BLOCK_LEN * 2);
    close(fd_out);
  }
  else if (extract_contents) {
    int fd_in = open(argv[2], O_RDONLY);
    if (fd_in < 0) {
      perror("open");
      exit(EXIT_FAILURE);
    }
    char *test = malloc(512);
    read(fd_in, test, 512);
    struct header info;
    read_archive_header(test, &info, strict);
  }
  return 0;
}
