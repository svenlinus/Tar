
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "integer.h"
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <limits.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#ifndef PATH_MAX
  #define PATH_MAX 2048
#endif
#define HEADER_LEN 512

void int_to_octal(int input, char *result, size_t size) {
  /* Converts an integer into and octal string and pads the start with 0's */
  int i, len;
  char *octal = malloc(size);
  sprintf(octal, "%o", input);
  if ((len = strlen(octal)) > size-1) {
    fprintf(stderr, "Can't create header: Octal %s too large", octal);
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < size - len - 1; i ++)
    result[i] = '0';
  strcpy(result + i, octal);
  result[size - 1] = '\0';
}

char *create_archive_header(char *file_path) {
  /* Writes the archive header for one file, returns pointer to header */
  char *header = calloc(HEADER_LEN, sizeof(char *));
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
      strncpy(prefix, file_path, i-1);
      if (i-1 < 155)
        prefix[i-1] = '\0';
      /* `header_name_field = file_path[i:]` */
      strncpy(header, file_path + i, 100);
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
  /* Convert to 4 digit octal (mask to ensure it is 4 digits) */
  int_to_octal(st.st_mode & 07777, mode_octal, 8);
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
    /* If uid_len > 7, use compress it and add to header */
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
  int_to_octal((S_ISREG(st.st_mode) ? st.st_size : 0), size, 12);
  strcpy(header + header_index, size);
  header_index += 12;

  /** Write mtime **/
  char mtime[12];
  int_to_octal(st.st_mtime, mtime, 12);
  strcpy(header + header_index, mtime);
  header_index += 12;

  /** Write typeflag **/
  /* skip chksum */
  header_index += 8;
  char type;
  if (S_ISDIR(st.st_mode)) 
    type = '5';
  else if (S_ISLNK(st.st_mode))
    type = '2';
  else
    type = '0';
  header[header_index] = type;
  header_index += 2;

  /** Write linkname **/
  header_index = 157;
  char readlink_buffer[100];
  ssize_t num_read = readlink(file_path, readlink_buffer, 100);
  if (num_read > 0) {
    strcpy(header + header_index, readlink_buffer);
    header_index += num_read;
    if (num_read != 100) {
      header[header_index] = '\0';
    }
  }

  /** Write magic **/
  header_index = 257;
  strcpy(header + header_index, "ustar");

  /** Write version **/
  header_index = 263;
  strcpy(header + header_index, "00");

  /** Write uname and gname **/
  header_index = 265;
  struct stat stat_buf;
  if (stat(file_path, &stat_buf) == -1) {
    perror("stat");
    exit(EXIT_FAILURE);
  }
  struct passwd *pw = getpwuid(stat_buf.st_uid);
  struct group *gr = getgrgid(stat_buf.st_gid);

  char *uname = pw->pw_name;
  char *gname = gr->gr_name;

  /* ask if the truncation is different */
  strncpy(header + header_index, uname, 32); 
  header_index = 297;
  strncpy(header + header_index, gname, 32);
  header_index = 329;

  /** Write prefix **/
  /* Location to write prefix: */
  header_index = 345;
  strcpy(header + header_index, prefix);

  /** Write chksum (should occur last) **/
  unsigned int sum = 0;
  char chksum[8];
  /* Location to write chksum: */
  header_index = 148;
  for (i = 0; i < HEADER_LEN; i ++) {
    sum += (unsigned char)header[i];
  }
  /* Add spaces (32) occupying the 8 chksum bytes (32 * 8) */
  sum += 256;
  int_to_octal(sum, chksum, 8);
  for (i = 0; i < 8; i ++) {
    header[header_index + i] = chksum[i];
  }

  return header;
}

void add_to_tarfile(char *to_add);
int isDirectory(char *path);

void directories_traversal(char *path) {
  /* level-order DFS of the directory */
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
    sprintf(child_path[i], "%s/%s", path, dir_read->d_name);

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
