#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char* argv[]) {
    while (true) {
        getppid();
    }
}
