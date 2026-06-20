#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <search_string>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat file_info;

    if (fstat(fd, &file_info) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t file_size = (size_t) file_info.st_size;
    size_t search_length = strlen(argv[2]);

    if (file_size == 0 || search_length == 0 ||
        search_length > file_size) {
        close(fd);
        return 0;
    }

    char *data = mmap(
        NULL,
        file_size,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0
    );

    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    for (size_t position = 0;
         position + search_length <= file_size;
         position++) {

        if (memcmp(
                data + position,
                argv[2],
                search_length
            ) == 0) {

            printf("%zu\n", position);
        }
    }

    if (munmap(data, file_size) == -1) {
        perror("munmap");
        close(fd);
        return 1;
    }

    if (close(fd) == -1) {
        perror("close");
        return 1;
    }

    return 0;
}