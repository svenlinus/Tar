#include <sys/stat.h>
#include "integer.h"

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