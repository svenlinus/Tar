#define BLOCK_SIZE 512
char *create_archive_header(char *file_path);
void traverse_directory(char *path, int output_fd);
void add_to_tarfile(char *to_add, int output_fd);
