#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include "functions.h"

void read_archive_header(char *header, struct header *info, bool strict) {
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
  for (i = 0; i < BLOCK_SIZE; i ++) {
    sum += (unsigned char)header[i];
  }
  /* If the actual sum doesn't match the header chksum, then the header is 
  corrupted */
  if (sum != info->chksum) {
    /* Throw error with expected (`sum`) vs actual (`info->chksum`) */
    fprintf(stderr, "Corrupt chksum\nexp: %d act: %d\n", sum, info->chksum);
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
    fprintf(stderr, "Corrupt magic\nexp: ustar act: %s\n", magic);
    exit(EXIT_FAILURE);
  }
  header_index += 6;

  /** Read version **/
  if (strict) {
    char version[3];
    strncpy(version, header + header_index, 2);
    version[2] = '\0';
    if (strcmp(version, "00")) {
      fprintf(stderr, "Corrupt version\nexp: 00 act: %s\n", version);
      exit(EXIT_FAILURE);
    }
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

  // printf("%s %d %d %d %d %d %d %c\n", info->name, info->stat.st_mode, info->stat.st_uid, info->stat.st_gid, (int)info->stat.st_size, (int)info->stat.st_mtime, info->chksum, info->typeflag);
  // printf("%s %s %s %s\n", info->linkname, info->uname, info->gname, info->prefix);
}

void print_entry(char *name, struct header info, bool verbose) {
  if (verbose) {
    /* Type */
    switch (info.typeflag) {
      case '5':
        printf("d");
        break;
      case '2':
        printf("1");
        break;
      default:
        printf("-");
        break;
    }
    /* Permissions */
    int i, mask = 0x100;
    char perms[9] = "rwxrwxrwx";
    for (i = 0; i < 9; i ++) {
      if (info.stat.st_mode & (mask >> i))
        printf("%c", perms[i]);
      else
        printf("-");
    }
    /* Uname/gname */
    printf(" %s/%s", info.uname, info.gname);
    /* Size */
    char size[8];
    char spaces[8];
    sprintf(size, "%d", (int)info.stat.st_size);
    int len = strlen(size);
    memset(spaces, ' ', 8 - len);
    printf("%s%s", spaces, size);
    /* mtime */
    struct tm* timeinfo = localtime(&info.stat.st_mtime);
    char buffer[20];  // Buffer to hold the formatted time
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", timeinfo);
    printf(" %s ", buffer);
  }
  /* Print name */
  printf("%s\n", name);
}

void list_contents(int fd, bool verbose, int num_files, char *files[]) {
  char header[BLOCK_SIZE];
  char path[257];
  int i;
  if (read(fd, header, BLOCK_SIZE) < 0) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  while (strlen(header) > 0) {
    struct header info;
    read_archive_header(header, &info, false);
    /* Add prefix to path */
    strncpy(path, info.prefix, 155);
    /* Add name to path */
    if (strlen(path) > 0)
      strcat(path, "/");
    strncat(path, info.name, 100);
    /* Add null terminator if necessary */
    if (path[255])
      path[256] = '\0';
    /* Print entry if desired */
    for (i = 0; i < num_files; i ++) {
      char temp[257];
      strcpy(temp, files[i]);
      /* Add '/' if user didn't include one */
      if (temp[strlen(temp) - 1] != '/')
        strcat(temp, "/");
      if (strncmp(temp, path, strlen(temp)) == 0) {
        print_entry(path, info, verbose);
        break;
      }
    }
    if (num_files == 0)
      print_entry(path, info, verbose);
    /* Seek to next header */
    int distance = info.stat.st_size ? info.stat.st_size / BLOCK_SIZE + 1 : 0;
    lseek(fd, distance * BLOCK_SIZE, SEEK_CUR);
    /* Read next header */
    if (read(fd, header, BLOCK_SIZE) < 0) {
      perror("read");
      exit(EXIT_FAILURE);
    }
  }
}

void extraction(char *tarfile_name, bool strict, bool verbose) {
  int curr_output_fd;
  struct header info;
  int curr_size;
  char *curr_name;
  char *curr_body_buffer;
  int num_bytes_read, num_bytes_written;
  int offset;
  int curr_type;
  struct stat sb;
  int perms = 0666;

  /* buffer to read in blocks at a time */
  char *read_buffer = (char *)malloc(BLOCK_SIZE);

  /* opening the tarfile to read from it*/
  int tar_fd = open(tarfile_name, O_RDONLY);
  if (tar_fd < 0) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  /* go through the tarfile one block at a time */
  do {
    /* read a block */
    num_bytes_read = read(tar_fd, read_buffer, BLOCK_SIZE);
    if (num_bytes_read < 0) {
      perror("read");
      exit(EXIT_FAILURE);
    }
    /* check that the first byte isn't null */
    if (read_buffer[0] != '\0') {

      /* get information about the dir/file */
      read_archive_header(read_buffer, &info, strict);
      curr_name = info.name;
      curr_size = info.stat.st_size;
      curr_type = info.typeflag;
      if (verbose) {
        printf("%s\n", curr_name);
      }
      /* case for a symlink */
      if (curr_type == 2) {
        curr_name = info.linkname;
      }
      /* case for a directory, continue */
      if (curr_type == 5) {
        /* making the dir */
        if (lstat(curr_name, &sb) == -1) {
          mkdir(curr_name, 0777);
        }
        continue;
      }
      /* open the un-tarred file */
      /* DO THE PERMS WORK??? NO */
      if (info.stat.st_mode & 0111) {
        perms = 0777;
      }
      curr_output_fd = open(curr_name, O_WRONLY | O_CREAT | O_TRUNC, perms);
      if (curr_size > 0) {
        curr_body_buffer = (char *)malloc(curr_size);
        num_bytes_read = read(tar_fd, curr_body_buffer, curr_size);
        if (num_bytes_read < 0) {
          perror("read");
          exit(EXIT_FAILURE);
        }
        num_bytes_written = write(curr_output_fd, curr_body_buffer, curr_size);
        if (num_bytes_written < 0) {
          perror("write");
          exit(EXIT_FAILURE);
        }
        free(curr_body_buffer);
        if (curr_size != BLOCK_SIZE) {
          offset = BLOCK_SIZE - (BLOCK_SIZE % curr_size);
          lseek(tar_fd, offset, SEEK_CUR);
        }
      }
      close(curr_output_fd);
    }
  } while(read_buffer[0] != '\0');
  close(tar_fd);
  free(read_buffer);
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