// For using O_DIRECT
#define _GNU_SOURCE

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define PART_SIZE (1024 * 1024 * 1024)
#define ACCESS_SIZE (64 * 1024 * 1024)

int main(int argc, char const* argv[]) {
    const char* program_name = argv[0];

    if (argc != 6) {
        fprintf(stderr, "Usage: %s <filename> <use kernel's acceleration (on/off)> <r/w> <access pattern (seq/rand)> <block size[KB]>\n",
                program_name);
        exit(EXIT_FAILURE);
    }

    const char* filename = argv[1];
    bool useKernelsAcceleration;
    if (strcmp(argv[2], "on") == 0) {
        useKernelsAcceleration = true;
    } else if (strcmp(argv[2], "off") == 0) {
        useKernelsAcceleration = false;
    } else {
        fprintf(stderr, "use kernel's acceleration should be 'on' or 'off': %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    bool is_write;
    if (strcmp(argv[3], "r") == 0) {
        is_write = false;
    } else if (strcmp(argv[3], "w") == 0) {
        is_write = true;
    } else {
        fprintf(stderr, "r/w should be 'r' or 'w': %s\n", argv[3]);
        exit(EXIT_FAILURE);
    }

    bool is_random;
    if (strcmp(argv[4], "seq") == 0) {
        is_random = false;
    } else if (strcmp(argv[4], "rand") == 0) {
        is_random = true;
    } else {
        fprintf(stderr, "access pattern should be 'seq' or 'rand': %s\n", argv[4]);
        exit(EXIT_FAILURE);
    }

    int block_size = atoi(argv[5]) * 1024;
    if (block_size <= 0) {
        fprintf(stderr, "block size should be > 0: %s\n", argv[5]);
        exit(EXIT_FAILURE);
    }

    int part_size = PART_SIZE;
    int access_size = ACCESS_SIZE;
    if (access_size % block_size != 0) {
        fprintf(stderr, "access size (%d) should be multiple of block size: %s\n", access_size, argv[5]);
        exit(EXIT_FAILURE);
    }

    int max_count = part_size / block_size;
    int count = access_size / block_size;

    int* offset = (int*)malloc(sizeof(int) * max_count);
    if (offset == NULL) {
        err(EXIT_FAILURE, "malloc() failed");
    }

    int oflag = O_RDWR | O_EXCL;
    if (!useKernelsAcceleration) {
        oflag |= O_DIRECT;
    }

    int fd = open(filename, oflag);
    if (fd == -1) {
        free(offset);
        err(EXIT_FAILURE, "open() failed");
    }

    for (int i = 0; i < max_count; i++) {
        offset[i] = i;
    }
    if (random) {
        for (int i = 0; i < max_count; i++) {
            int j = rand() % max_count;
            int tmp = offset[i];
            offset[i] = offset[j];
            offset[j] = tmp;
        }
    }

    int sector_size;
    if (ioctl(fd, BLKSSZGET, &sector_size) == -1) {
        free(offset);
        close(fd);
        err(EXIT_FAILURE, "ioctl() failed");
    }

    char* buff;
    int e = posix_memalign((void**)&buff, sector_size, block_size);
    if (e) {
        errno = e;
        free(offset);
        close(fd);
        err(EXIT_FAILURE, "posix_memalign() failed");
    }
    for (int i = 0; i < count; i++) {
        if (lseek(fd, offset[i] * block_size, SEEK_SET) == -1) {
            free(buff);
            free(offset);
            close(fd);
            err(EXIT_FAILURE, "lseek() failed");
        }
        if (is_write) {
            ssize_t ret = write(fd, buff, block_size);
            if (ret == -1) {
                free(buff);
                free(offset);
                close(fd);
                err(EXIT_FAILURE, "write() failed");
            }
        } else {
            ssize_t ret = read(fd, buff, block_size);
            if (ret == -1) {
                free(buff);
                free(offset);
                close(fd);
                err(EXIT_FAILURE, "read() failed");
            }
        }
    }

    if (fdatasync(fd) == -1) {
        free(buff);
        free(offset);
        close(fd);
        err(EXIT_FAILURE, "fdatasync() failed");
    }

    free(buff);
    free(offset);
    if (close(fd) == -1) {
        err(EXIT_FAILURE, "close() failed");
    }

    return 0;
}
