#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

void traverse_directory(char *path) {
    DIR *dir;
    struct dirent *dir_read;
    struct stat stat_buffer;
    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    while((dir_read = readdir(dir)) != NULL) {
        /* skipping over self and parent */
        if (strcmp(dir_read->d_name, ".") == 0 
        || strcmp(dir_read->d_name, "..") == 0) {
            continue;
        }

        /* getting the path of the current child */
        char new_path[512];
        strcpy(new_path, path);
        strcat(new_path, "/");
        strcat(new_path, dir_read->d_name);

        /* printing the curr child */
        printf("%s\n", new_path);

        /* recurses with the new path */
        if (lstat(new_path, &stat_buffer) < 0) {
            perror("lstat");
            exit(EXIT_FAILURE);
        }
        if (S_ISDIR(stat_buffer.st_mode)) {
            traverse_directory(new_path);
        }
    }
    closedir(dir);
}

int main(int argc, char **argv) {
    traverse_directory("traversal.dSYM");
    return 0;
}
