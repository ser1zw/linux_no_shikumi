#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define BUFFER_SIZE (100 * 1024 * 1024)
#define PAGE_SIZE 4096
#define COMMAND_SIZE 4096

char command[COMMAND_SIZE];

void child(char *p) {
    puts("*** child ps info before memory access ***:");
    fflush(stdout);
    snprintf(command, COMMAND_SIZE, "ps -o pid,comm,vsz,rss,min_flt,maj_flt | grep -E '^\\s*(PID|%d)'", getpid());
    system(command);
    puts("*** free memory info before memory access ***:");
    fflush(stdout);
    system("free");

    for (int i = 0; i < BUFFER_SIZE; i += PAGE_SIZE) {
        p[i] = 0;
    }

    puts("*** child ps info after memory access ***:");
    fflush(stdout);
    system(command);
    puts("*** free memory info after memory access ***:");
    fflush(stdout);
    system("free");
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
    char *p = malloc(BUFFER_SIZE);
    if (p == NULL) {
        err(EXIT_FAILURE, "malloc() failed");
    }

    for (int i = 0; i < BUFFER_SIZE; i += PAGE_SIZE) {
        p[i] = 0;
    }
    puts("*** free memory info before fork ***:");
    fflush(stdout);
    system("free");

    pid_t ret = fork();
    if (ret == -1) {
        err(EXIT_FAILURE, "fork() failed");
    }

    if (ret == 0) {
        child(p);
    } else {
        wait(NULL);
    }

    return 0;
}
