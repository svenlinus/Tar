#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include "decode.h"
#include "integer.h"
#include "encode.h"

void read_archive_header(char *header, struct header *info) {
  /* Extracts desired header fields and stores the data in `info` */
  int header_index = 0;
  int i;

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

  /** Read size **/
  char size[12];
  strncpy(size, header + header_index, 12);
  info->stat.st_size = octal_to_int(size, 12);
  header_index += 12;

  /** Read mtime **/
  char mtime[12];
  strncpy(mtime, header + header_index, 12);
  info->stat.st_mtime = octal_to_int(mtime, 12);
  header_index += 12;

  /** Read chksum **/
  char chksum[8];
  strncpy(chksum, header + header_index, 8);
  info->chksum = octal_to_int(chksum, 8);
  /* Fill chksum field with spaces */
  memset(header + header_index, ' ', 8);
  header_index += 8;
  /* Recalculate chksum */
  int sum = 0;
  for (i = 0; i < HEADER_LEN; i ++) {
    sum += (unsigned char)header[i];
  }
  /* If the actual sum doesn't match the header chksum, then the header is 
  corrupted */
  if (sum != info->chksum) {
    /* Throw error with expected (`sum`) vs actual (`info->chksum`) */
    fprintf(stderr, "Corrupt chksum\ne: %d a: %d\n", sum, info->chksum);
    exit(EXIT_FAILURE);
  }

  /** Read typeflag **/
  info->typeflag = header[header_index];
  header_index += 1;

  /** Read linkname **/
  strncpy(info->linkname, header + header_index, 100);
  header_index += 100;

  /** Read magic **/
  char magic[6];
  strncpy(magic, header + header_index, 6);
  if (strcmp(magic, "ustar")) {
    fprintf(stderr, "Corrupt magic\ne: ustar a: %s\n", magic);
    exit(EXIT_FAILURE);
  }
  header_index += 6;

  /** Read version **/
  char version[3];
  strncpy(version, header + header_index, 2);
  version[2] = '\0';
  if (strcmp(version, "00")) {
    fprintf(stderr, "Corrupt version\ne: 00 a: %s\n", version);
    exit(EXIT_FAILURE);
  }
  header_index += 2;

  /** Read uname **/
  strncpy(info->uname, header + header_index, 32);
  header_index += 32;

  /** Read gname **/
  strncpy(info->gname, header + header_index, 32);
  header_index += 32;

  /** Read prefix **/
  header_index = 345;
  strncpy(info->prefix, header + header_index, 155);
  header_index += 155;

  printf("%s %d %d %d %d %d %d %c\n", info->name, info->stat.st_mode, info->stat.st_uid, info->stat.st_gid, (int)info->stat.st_size, (int)info->stat.st_mtime, info->chksum, info->typeflag);
  printf("%s %s %s %s\n", info->linkname, info->uname, info->gname, info->prefix);
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