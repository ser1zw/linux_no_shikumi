#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <err.h>

#define BUFFER_SIZE 1000
#define ALLOC_SIZE (100 * 1024 * 1024)

#define FILENAME "textfile.txt"

void close_file(int fd) {
    if (close(fd) == -1) {
        warn("close() failed");
    }
}

/**
 * Create a text file `textfile.txt` before running this program.
 * $ echo 'hello' > textfile.txt
 */
int main(int argc, char const *argv[]) {
    char command[BUFFER_SIZE];
    char overwirte_data[] = "HELLO";

    pid_t pid = getpid();
    snprintf(command, BUFFER_SIZE, "cat /proc/%d/maps", pid);

    puts("*** memory map before mapping file ***");
    fflush(stdout);
    system(command);

    int fd = open(FILENAME, O_RDWR);
    if (fd == -1) {
        err(EXIT_FAILURE, "open() failed");
    }

    char *file_contents = mmap(NULL, ALLOC_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (file_contents == (void*) -1) {
        warn("mmap() failed");
        close_file(fd);
        exit(EXIT_FAILURE);
    }

    puts("");
    printf("*** successed to map file: address = %p; size = %x ***\n", file_contents, ALLOC_SIZE);
    fflush(stdout);
    system(command);

    puts("");
    printf("*** file contents before overwrite mapped region: [%s] ***\n", file_contents);

    memcpy(file_contents, overwirte_data, strlen(overwirte_data));

    puts("");
    printf("*** overwritten mapped region with: [%s] ***\n", file_contents);

    if (munmap(file_contents, ALLOC_SIZE)) {
        warn("munmap() failed");
    }

    close_file(fd);

    return 0;
}
