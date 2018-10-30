#include <stdio.h>
#include <time.h>
#include <unistd.h>

int main() {
	while (1) {
		printf("%ld\n", time(NULL));
		sleep(1);
	}
	return 0;
}
