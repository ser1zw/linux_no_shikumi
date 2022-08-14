#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

void child() {
    printf("Child process: pid = %d\n", getpid());
    exit(EXIT_SUCCESS);
}

void parent(pid_t child_pid) {
    printf("Parent process: pid = %d, child pid = %d\n", getpid(), child_pid);
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
    pid_t ret;

    ret = fork();
    if (ret == -1) {
        err(EXIT_FAILURE, "fork() failed");
    }
    if (ret == 0) {
        child();
    } else {
        parent(ret);
    }

    return 0;
}
