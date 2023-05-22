#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#define BLOCK_SIZE 512
typedef enum { false, true } bool;

uint32_t extract_special_int(char *where, int len);
int insert_special_int(char *where, size_t size, int32_t val);
char *create_archive_header(char *file_path);
void traverse_directory(char *path, int output_fd, bool verbose);
void add_to_tarfile(char *to_add, int output_fd);
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
long int octal_to_int(char *input, size_t size);
void read_archive_header(char *header, struct header *info, bool strict);
void list_contents(int fd, bool verbose, int num_files, char *files[]);