#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#define BLOCK_SIZE 512
#define PREF_LEN 155
#define NAME_LEN 100
#define PATH_LEN 256
#define ALL_PERMS 0777
typedef enum { false, true } bool;

uint32_t extract_special_int(char *where, int len);
int insert_special_int(char *where, size_t size, int32_t val);
char *create_archive_header(char *file_path);
void traverse_directory(char *path, int output_fd, bool verbose);
void add_to_tarfile(char *to_add, int output_fd);
struct header {
  struct stat stat;
  char name[NAME_LEN];
  int chksum;
  char typeflag;
  char linkname[NAME_LEN];
  char uname[32];
  char gname[32];
  char prefix[PREF_LEN];
};
long int octal_to_int(char *input);
void read_archive_header(char *header, struct header *info, bool strict);
void list_contents(int fd, bool verbose, int num_files, char *files[]);
void extraction(char *tarfile_name, bool strict, bool verbose, char *spec);
void maybe_create_dir(char *new_dir);