#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void child() {
    char* args[] = { "/bin/echo", "hello", NULL };
    printf("Child process: pid = %d\n", getpid());
    fflush(stdout);
    execve("/bin/echo", args, NULL);

    // Should not reach here
    exit(EXIT_FAILURE);
}

void parent(pid_t child_pid) {
    printf("Parent process: pid = %d, child pid = %d\n", getpid(), child_pid);
    exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[]) {
    pid_t ret;
    ret = fork();
    if (ret == -1) {
        fprintf(stderr, "fork() failed");
        exit(EXIT_FAILURE);
    } else if (ret == 0) {
        child();
    } else {
        parent(ret);
    }

    return 0;
}
