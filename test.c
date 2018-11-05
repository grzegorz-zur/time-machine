#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main() {
    time_t t;
    do {
        t = time(NULL);
        printf("%ld\t%s", t, ctime(&t));
    } while (!sleep(1));
    return 0;
}
