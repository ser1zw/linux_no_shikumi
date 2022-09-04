#include <stdio.h>

int main(int argc, char const *argv[]) {
    int *p = NULL;
    puts("before invalid access");
    *p = 0;
    puts("after invalid access");
    return 0;
}
