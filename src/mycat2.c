#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

size_t get_page_size() {
    size_t page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        perror("Error when getting page size.");
        exit(1);
    }
    return page_size;
}

size_t io_blocksize() {
    return get_page_size();
}

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("Error when opening file.");
        exit(1);
    }

    size_t buffer_size = io_blocksize();
    char *buffer = malloc(buffer_size);
    if (buffer == NULL) {
        perror("Error when allocating buffer.");
        close(fd);
        exit(1);
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {
        if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
            perror("Error when writing to stdout.");
            free(buffer);
            close(fd);
            exit(1);
        }
    }

    if (bytes_read == -1) {
        perror("Error when reading from file.");
        free(buffer);
        close(fd);
        exit(1);
    }

    free(buffer);
    close(fd);
    return 0;
}