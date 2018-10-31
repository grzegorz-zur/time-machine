#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main() {
    do {
        printf("%ld\n", time(NULL));
    } while (!sleep(1));
    return 0;
}
