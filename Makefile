.PHONY : clean

CFLAGS = -g -fPIC -pedantic -Wall -Wextra

all: timemachine.so

clean:
	rm -f timemachine.so

timemachine.so : timemachine.c
	$(CC) $(CFLAGS) $< -o $@ -shared -Wl,--no-as-needed -ldl
