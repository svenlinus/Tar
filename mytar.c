#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "encode.h"
#include "decode.h"
#include "integer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>

typedef enum { false, true } bool;
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
  if (!create_archive && !print_contents && !extract_contents)
    usage();

  /* passing the path through the traversal function */
  // path = argv[3];
  // printf("this is the path: %s\n", path);
  // directories_traversal(path);

  if (create_archive) {
    char *header = create_archive_header(argv[3]);
    int fd_out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    write(fd_out, header, HEADER_LEN);
  }

  /** Testing decode **/
  if (extract_contents) {
    int fd_in = open(argv[2], O_RDONLY);
    char *test = malloc(512);
    read(fd_in, test, 512);
    struct header info;
    read_archive_header(test, &info);
  }
  return 0;
}
