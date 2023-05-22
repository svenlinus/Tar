#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "functions.h"
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
  int j;
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

  if (print_contents) {
    int fd_in = open(argv[2], O_RDONLY);
    if (fd_in < 0) {
      perror("open");
      exit(EXIT_FAILURE);
    }
    list_contents(fd_in, verbose, argc - 3, argc > 3 ? argv + 3 : NULL);
  }
  else if (create_archive) {
    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd_out < 0) {
      perror("open");
      exit(EXIT_FAILURE);
    }
    for (i = 3; i < argc; i ++) {
      if (lstat(argv[i], &sb) < 0) {
        fprintf(stderr, "No such file of directory: %s\n", argv[i]);
        continue;
      }
      if (!(S_ISDIR(sb.st_mode))) {
        add_to_tarfile(argv[i], fd_out);
      }
      else if (S_ISDIR(sb.st_mode)) {
        char temp[256];
        strcpy(temp, argv[i]);
        /* Add '/' if user didn't include one */
        if (temp[strlen(temp) - 1] != '/')
          strcat(temp, "/");
        if (verbose)
          printf("%s\n", temp);
        add_to_tarfile(temp, fd_out);
        traverse_directory(temp, fd_out, verbose);
      }
    }
    char *two_null_blocks = (char *)calloc(BLOCK_SIZE * 2, 1);
    write(fd_out, two_null_blocks, BLOCK_SIZE * 2);
    close(fd_out);
  }
  else if (extract_contents) {
    if (argc > 3) {
      int num_specs = argc - 3;
      char spec[num_specs][256];
      for (i=0; i<num_specs; i++) {
        for (j=0; j<256; j++) {
          spec[i][j] = '\0';
        }
      }
      for (i=0; i<num_specs; i++) {
        strcpy(spec[i], argv[i+3]);
        extraction(argv[2], strict, verbose, spec[i]);
      }
    }
    else {
      extraction(argv[2], strict, verbose, NULL);
    }
  }
  return 0;
}
