#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

// CACHE_LINE_SIZE should be same as /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size
#define CACHE_LINE_SIZE 64
#define NLOOP (4 * 1024UL * 1024 * 1024)
#define NSECS_PER_SEC 1000000000UL

inline long diff_nsec(struct timespec before, struct timespec after) {
    return (after.tv_sec * NSECS_PER_SEC + after.tv_nsec) - (before.tv_sec * NSECS_PER_SEC + before.tv_nsec);
}

/**
 * $ cc -O3 -o cache cache.c
 * $ ./cache 4
 */
int main(int argc, char const *argv[]) {
    const char *program_name = argv[0];

    if (argc != 2) {
        fprintf(stderr, "usage: %s <size[KB]>\n", program_name);
        exit(EXIT_FAILURE);
    }

    register int size = atoi(argv[1]);
    if (size <= 0) {
        fprintf(stderr, "size should be >= 1: %d\n", size);
        exit(EXIT_FAILURE);
    }
    size *= 1024;

    char *buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (buffer == (void *)-1) {
        err(EXIT_FAILURE, "mmap() failed");
    }

    struct timespec before, after;
    clock_gettime(CLOCK_MONOTONIC, &before);

    for (int i = 0; i < NLOOP / (size / CACHE_LINE_SIZE); i++) {
        for (long j = 0; j < size; j += CACHE_LINE_SIZE) {
            buffer[j] = 0;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &after);

    printf("%d %f\n", size, (double)diff_nsec(before, after) / NLOOP);

    if (munmap(buffer, size) == -1) {
        err(EXIT_FAILURE, "munmap() failed");
    }

    return 0;
}
