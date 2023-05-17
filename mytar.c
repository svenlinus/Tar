#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "encode.h"
typedef enum { false, true } bool;
extern char *optarg;

void usage() {
  fprintf(stderr, "usage: mytar [ctxvS]f tarfile [ path [ ... ] ]\n");
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  char opt;
  int i = 0;
  /* Option flags */
  char *output = NULL;
  bool create_archive = false, 
       print_contents = false, 
       extract_contents = false, 
       verbose = false, 
       strict = false;
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
  printf("%s %d %d %d %d %d\n", output, create_archive, print_contents, extract_contents, verbose, strict);
  if (!output)
    usage();
  if (!create_archive && !print_contents && !extract_contents)
    usage();
  char *header = create_archive_header(argv[2]);
  for (i = 0; i < 512; i ++) {
    printf("%c", header[i]);
  }
  printf("\n");
  return 0;
}