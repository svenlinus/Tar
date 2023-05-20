#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

struct header {
  struct stat stat;
  char name[100];
  int chksum;
  char typeflag;
  char linkname[100];
  char uname[32];
  char gname[32];
  char prefix[155];
};

long int octal_to_int(char *input, size_t size) {
  /* Converts an octal string into integer */
  unsigned int i, octal;
  unsigned long int result = 0;
  for (i = 0; i < size-1; i ++) {
    octal = (input[i] - '0') & 07;
    result |= octal << ((size - 2 - i) * 3);
  }
  return result;
}

void read_archive_header(char *header, struct header *info) {
  int header_index = 0;

  /** Read name **/
  strncpy(info->name, header, 100);
  header_index += 100;

  /** Read mode **/
  char mode[8];
  strncpy(mode, header + header_index, 8);
  
}