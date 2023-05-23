#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include "functions.h"
#include <errno.h>

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
  info->stat.st_mode = octal_to_int(mode);
  header_index += 8;

  /** Read UID **/
  char id[8];
  int len = strlen(strncpy(id, header + header_index, 8));
  if (len < 7)
    info->stat.st_uid = extract_special_int(header + header_index, 8);
  else 
    info->stat.st_uid = octal_to_int(id);
  header_index += 8;

  /** Read GID **/
  strncpy(id, header + header_index, 8);
  info->stat.st_gid = octal_to_int(id);
  header_index += 8;

  /** Read size **/
  char size[12];
  strncpy(size, header + header_index, 12);
  info->stat.st_size = octal_to_int(size);
  header_index += 12;

  /** Read mtime **/
  char mtime[12];
  strncpy(mtime, header + header_index, 12);
  info->stat.st_mtime = octal_to_int(mtime);
  header_index += 12;

  /** Read chksum **/
  char chksum[8];
  strncpy(chksum, header + header_index, 8);
  info->chksum = octal_to_int(chksum);
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
  if (strict) {
    if (strcmp(magic, "ustar")) {
      fprintf(stderr, "Corrupt magic\nexp: ustar act: %s\n", magic);
      exit(EXIT_FAILURE);
    } 
  } else if (strncmp(magic, "ustar", 5)) {
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
}

void print_entry(char *name, struct header info, bool verbose) {
  if (verbose) {
    /* Type */
    switch (info.typeflag) {
      case '5':
        printf("d");
        break;
      case '2':
        printf("l");
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
    printf(" %s/%s ", info.uname, info.gname);
    /* Size */
    char size[8];
    char spaces[8];
    for (i = 0; i < 8; i ++)
      spaces[i] = '\0';
    sprintf(size, "%d", (int)info.stat.st_size);
    int len = strlen(size);
    memset(spaces, ' ', 8 - len);
    printf("%s%s", spaces, size);
    /* mtime */
    struct tm* timeinfo = localtime(&info.stat.st_mtime);
    char buffer[20];
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
      if (strcmp(files[i], path) == 0) {
        print_entry(path, info, verbose);
        break;
      }
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

void extraction(char *tarfile_name, bool strict, bool verbose, char *spec) {

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
  int null_block;
  int curr_mode;
  int i;
  int skip_flag = 0;
  char new_dirs[256];

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
    skip_flag = 0;
    /* read a block */
    num_bytes_read = read(tar_fd, read_buffer, BLOCK_SIZE);
    /* printf("num_bytes_read: %d\n", num_bytes_read); */
    if (num_bytes_read < 0) {
      perror("read");
      exit(EXIT_FAILURE);
    }
    /* check that the first byte isn't null */
    if (read_buffer[0] != '\0') {

      /* get information about the dir/file */
      read_archive_header(read_buffer, &info, strict);
      curr_name = info.name;

      curr_mode = info.stat.st_mode;
      if (spec != NULL) {
        if (S_ISDIR(curr_mode) && spec[strlen(spec) - 1] != '/')
          strcat(spec, "/");
        if (strncmp(spec, curr_name, strlen(spec)) != 0) {
          skip_flag = 1;
        }
      }

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
      if (info.stat.st_mode & 0111) {
        perms = 0777;
      }
      if (curr_size > 0) {
        curr_body_buffer = (char *)malloc(curr_size);
        num_bytes_read = read(tar_fd, curr_body_buffer, curr_size);
        if (num_bytes_read < 0) {
          perror("read");
          exit(EXIT_FAILURE);
        }
        /* checks if we should be writing or not */
        if (skip_flag == 0) {
          /* iterate through the curr_name
           * when I reach a /, create that directory */
          for (i=0; i<256; i++) {
            new_dirs[i] = '\0';
          }
          i=0;
          while (curr_name[i] != '\0') {
            if (curr_name[i] != '/') {
              new_dirs[i] = curr_name[i];
            }
            else {
              new_dirs[i] = '/';
              maybe_create_dir(new_dirs);
            }
            i++;
          }
          curr_output_fd = open(curr_name, O_WRONLY | O_CREAT | O_TRUNC, perms);
          if (curr_output_fd == -1) {
            fprintf(stderr, "curr_name: %s\nperms: %o\n", curr_name, perms);
            fprintf(stderr, "errno: %s\n", strerror(errno));
          }
          num_bytes_written = write(curr_output_fd, curr_body_buffer, curr_size);
          if (num_bytes_written < 0) {
            perror("write");
            exit(EXIT_FAILURE);
          }
          close(curr_output_fd);
        }
        free(curr_body_buffer);
        if (curr_size != BLOCK_SIZE) {
          offset = BLOCK_SIZE - (curr_size % BLOCK_SIZE);
          lseek(tar_fd, offset, SEEK_CUR);
        }
      }
    }
    null_block = 1;
    for (i=0; i<BLOCK_SIZE; i++) {
      if (read_buffer[i] != '\0') {
        null_block = 0;
        break;
      }
    }
  } while(null_block == 0);
  close(tar_fd);
  free(read_buffer);
}

int octal_len(char *octal) {
  /* Calculates length of octal number. Non digit terminating */
  int len = 0;
  while (octal[len] - '0' >= 0 && octal[len] - '0' <= 9)
    len ++;
  return len;
}

long int octal_to_int(char *input) {
  /* Converts an octal string into integer */
  unsigned int i, octal;
  unsigned long int result = 0;
  int len = octal_len(input);
  for (i = 0; i < len; i ++) {
    octal = (input[i] - '0') & 07;
    result |= octal << ((len - 1 - i) * 3);
  }
  return result;
}

void maybe_create_dir(char *new_dir) {
  struct stat sb;
  if (lstat(new_dir, &sb) < 0) {
    mkdir(new_dir, 0777);
  }
}
