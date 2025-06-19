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

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("Error when opening file.");
        exit(1);
    }

    char buffer; ssize_t bytes_read;
    while ((bytes_read = read(fd, &buffer, 1)) > 0) {
        if (write(STDOUT_FILENO, &buffer, 1) == -1) {
            perror("Error when writing to stdout.");
            close(fd);
            exit(1);
        }
    }

    if (bytes_read == -1) {
        perror("Error when reading from file.");
        close(fd);
        exit(1);
    }

    close(fd);
    return 0;
}