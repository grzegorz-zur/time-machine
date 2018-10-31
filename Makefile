.PHONY : clean

CFLAGS = -g -fPIC -Wall -Wextra -Wpedantic

all : timemachine.so 

clean :
	rm -f timemachine.so test

timemachine.so : timemachine.c
	$(CC) $(CFLAGS) $< -o $@ -shared -pthread -Wl,--no-as-needed -ldl

test : test.c
	$(CC) $(CFLAGS) $< -o $@ 
