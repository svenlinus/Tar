#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "integer.h"

void mode_to_octal(mode_t mode, char *result) {
  /* Converts an integer `mode_t` into and octal string */
  mode_t mask = 07000;
  int i;
  short int perm;
  result[0] = '0';
  result[1] = '0';
  result[2] = '0';
  for (i = 0; i < 4; i++) {
    perm = (mode & mask) >> ((3-i) * 3);
    result[i + 3] = perm + 48;
    mask >>= 3;
  }
  result[7] = '\0';
}

char *create_archive_header(char *file_path) {
  /* Writes the archive header for one file, returns pointer to header */
  char *header = calloc(512, sizeof(char *));
  char prefix[155];
  int header_index = 0;
  int i;

  /** Write file name **/
  int path_len;
  if ((path_len = strlen(file_path)) > 255) {
      fprintf(stderr, "File path %s too long", file_path);
      exit(EXIT_FAILURE);
  }
  /* If `file_path` can't fit in name field (100 bytes), include the end of 
  `file_path` in name and write the rest in `prefix` */
  if (path_len > 100) {
    i = path_len - 100;
    /* Find the first '/' within 100 characters from the end */
    while (i < path_len && file_path[i++] != '/')
      ;
    if (i < path_len-1) {
      /* `prefix = file_path[0:i]` */
      strncpy(prefix, file_path, i);
      /* `header_name_field = file_path[i:]` */
      strcpy(header, file_path + i);
    } else {
      fprintf(stderr, "Can't create header: File name %s too long", file_path);
      exit(EXIT_FAILURE);
    }
  } else {
    strcpy(header, file_path);
  }
  header_index += 100;

  /** Write file mode **/
  struct stat st;
  char mode_octal[8];
  /* Get `file_path` stats */
  if (lstat(file_path, &st)) {
    perror("lstat");
    exit(EXIT_FAILURE);
  }
  mode_to_octal(st.st_mode, mode_octal);
  strcpy(header + header_index, mode_octal);
  header_index += 8;

  /** Write UID and GID **/
  char uid[8], gid[8];
  int uid_len, gid_len;
  /* Convert integers st_uid/st_gid to octal strings */
  sprintf(uid, "%o", st.st_uid);
  sprintf(gid, "%o", st.st_gid);
  if ((uid_len = strlen(uid)) < 8) {
    /* Pad the start of uid with zeroes */
    for (i = 0; i < 7-uid_len; i ++)
      header[header_index + i] = '0';
    /* Add uid to header */
    strcpy(header + header_index + i, uid);
    header[header_index + i + uid_len] = '\0';
  } else {
    /* If uid_lem > 7, use compress it and add to header */
    insert_special_int(header + header_index, 8, st.st_uid);
  }
  header_index += 8;
  if ((gid_len = strlen(gid)) < 8) {
    /* Pad the start of gid with zeroes */
    for (i = 0; i < 7-gid_len; i ++)
      header[header_index + i] = '0';
    /* Add gid to header */
    strcpy(header + header_index + i, gid);
    header[header_index + i + gid_len] = '\0';
    header_index += 8;
  } else {
      fprintf(stderr, "Can't create header: GID %s too long", gid);
      exit(EXIT_FAILURE);
  }

  /** Write size **/
  char size[12];
  
  return header;
}