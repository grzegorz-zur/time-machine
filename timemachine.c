#define _GNU_SOURCE

#include <errno.h>
#include <dlfcn.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define OFFSET "TIME_MACHINE_OFFSET"

static void init(void) __attribute__((constructor));

time_t offset = 0;

static void init(void) {
	char* offset_text = getenv(OFFSET);
	if (offset_text != NULL) {
		long offset_value = strtol(offset_text, NULL, 10);
		int err = errno;
		if (!err) {
			offset = offset_value;
		} else {
			fprintf(stderr, "timemachine: invalid value of offset: %s", offset_text);
		}
	}
}

time_t time(time_t *tloc) {
	static time_t (*func)(time_t*);
	if(func == NULL) {
		func = (time_t (*)(time_t*)) dlsym(RTLD_NEXT, "time");
	}
	if (tloc != NULL) {
		*tloc += offset;
	}
	time_t res = func(tloc) + offset;
	return res;
}

int clock_gettime(clockid_t clk_id, struct timespec *res) {
	static int (*func)(clockid_t, struct timespec*);
	if(func == NULL) {
		func = (int (*)(clockid_t, struct timespec*)) dlsym(RTLD_NEXT, "clock_gettime");
	}
	int err = func(clk_id, res);
	if (res != NULL) {
		res->tv_sec += offset;
	}
	return err;
}

int gettimeofday(struct timeval *tv, struct timezone *tz) {
	static int (*func)(struct timeval*, struct timezone*);
	if(func == NULL) {
		func = (int (*)(struct timeval*, struct timezone*)) dlsym(RTLD_NEXT, "gettimeofday");
	}
	int err = func(tv, tz);
	tv->tv_sec += offset;
	return err;
}

