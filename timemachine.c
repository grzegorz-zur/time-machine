#define _GNU_SOURCE

#include <errno.h>
#include <dlfcn.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define OFFSET "TIME_MACHINE_OFFSET"

time_t offset = 0;

static void __attribute__((constructor)) init(void) {
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

typedef time_t (*func_time_t)(time_t*);

time_t time(time_t *tloc) {
	static func_time_t func;
	if (func == NULL) {
		func = (func_time_t) dlsym(RTLD_NEXT, "time");
	}
	if (tloc != NULL) {
		*tloc += offset;
	}
	time_t res = func(tloc) + offset;
	return res;
}

typedef int (*func_clock_gettime_t)(clockid_t, struct timespec*);

int clock_gettime(clockid_t clk_id, struct timespec *res) {
	static func_clock_gettime_t func;
	if(func == NULL) {
		func = (func_clock_gettime_t) dlsym(RTLD_NEXT, "clock_gettime");
	}
	int err = func(clk_id, res);
	if (res != NULL) {
		res->tv_sec += offset;
	}
	return err;
}

typedef int (*func_gettimeofday_t)(struct timeval*, struct timezone*);

int gettimeofday(struct timeval *tv, struct timezone *tz) {
	static func_gettimeofday_t func;
	if(func == NULL) {
		func = (func_gettimeofday_t) dlsym(RTLD_NEXT, "gettimeofday");
	}
	int err = func(tv, tz);
	tv->tv_sec += offset;
	return err;
}

