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

size_t get_file_block_size(int fd) {
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error when getting file size.");
        exit(1);
    }

    // 检查是不是 2 的幂次
    if (st.st_blksize & (st.st_blksize - 1) != 0) {
        perror("File block size is not a power of 2.");
        exit(1);
    }
    return st.st_blksize;
}

size_t gcd(size_t a, size_t b) {
    return b == 0 ? a : gcd(b, a % b);
}

size_t io_blocksize(int fd) {
    return 64 * get_page_size() * get_file_block_size(fd) / gcd(get_page_size(), get_file_block_size(fd));
}

char* align_alloc(size_t size) {
    size_t page_size = get_page_size();
    void *ptr;
    if (posix_memalign(&ptr, page_size, size) != 0) {
        perror("Error when allocating memory.");
        exit(1);
    }
    return ptr;
}

void align_free(void *ptr) {
    free(ptr);
}

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("Error when opening file.");
        exit(1);
    }

    posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
    posix_fadvise(fd, 0, 0, POSIX_FADV_NOREUSE);

    size_t buffer_size = io_blocksize(fd);
    char *buffer = align_alloc(buffer_size);
    if (buffer == NULL) {
        perror("Error when allocating buffer.");
        close(fd);
        exit(1);
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, buffer_size)) > 0) {
        if (write(STDOUT_FILENO, buffer, bytes_read) == -1) {
            perror("Error when writing to stdout.");
            align_free(buffer);
            close(fd);
            exit(1);
        }
    }

    if (bytes_read == -1) {
        perror("Error when reading from file.");
        align_free(buffer);
        close(fd);
        exit(1);
    }

    align_free(buffer);
    close(fd);
    return 0;
}