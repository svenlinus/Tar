#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "decode.h"
#include "integer.h"

void read_archive_header(char *header, struct header *info) {
  int header_index = 0;
  int i = 0;

  /** Testing purposes **/
  int c;
  while ((c = header[i++]) != EOF && i < 10000)
    ;
  printf("%d\n", i);

  /** Read name **/
  strncpy(info->name, header, 100);
  header_index += 100;

  /** Read mode **/
  char mode[8];
  strncpy(mode, header + header_index, 8);
  info->stat.st_mode = octal_to_int(mode, 8);
  header_index += 8;

  /** Read UID **/
  char id[8];
  int len = strlen(strncpy(id, header + header_index, 8));
  if (len < 7)
    info->stat.st_uid = extract_special_int(header + header_index, 8);
  else 
    info->stat.st_uid = octal_to_int(id, 8);
  header_index += 8;

  /** Read GID **/
  strncpy(id, header + header_index, 8);
  info->stat.st_gid = octal_to_int(id, 8);
  header_index += 8;
  printf("%s %d %d %d\n", info->name, info->stat.st_mode, info->stat.st_uid, info->stat.st_gid);
}

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