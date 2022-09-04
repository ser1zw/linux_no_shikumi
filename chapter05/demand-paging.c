#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <err.h>

#define BUFFER_SIZE (100 * 1024 * 1024)
#define NCYCLE 10
#define PAGE_SIZE 4096

int main(int argc, char const *argv[]) {

    time_t t = time(NULL);
    char* s = ctime(&t);
    printf("%.*s: Before allocation. Press Enter key\n", (int)(strlen(s) - 1), s);
    getchar();

    char* p = malloc(BUFFER_SIZE);
    if (p == NULL) {
        err(EXIT_FAILURE, "malloc() failed");
    }

    t = time(NULL);
    s = ctime(&t);
    printf("%.*s: Allocated %dMB. Press Enter key\n", (int)(strlen(s) - 1), s, BUFFER_SIZE / (1024 * 1024));
    getchar();

    for (int i = 0; i < BUFFER_SIZE; i += PAGE_SIZE) {
        p[i] = 0;
        int cycle = i / (BUFFER_SIZE / NCYCLE);
        if (cycle != 0 && i % ((BUFFER_SIZE / NCYCLE)) == 0) {
            t = time(NULL);
            s = ctime(&t);
            printf("%.*s: touched %dMB\n", (int)(strlen(s) - 1), s, i / (1024 * 1024));
            sleep(1);
        }
    }

    t = time(NULL);
    s = ctime(&t);
    printf("%.*s: touched %dMB. Press Enter key\n", (int)(strlen(s) - 1), s, BUFFER_SIZE / (1024 * 1024));
    getchar();

    free(p);

    return 0;
}
