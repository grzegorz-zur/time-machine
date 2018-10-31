#define _GNU_SOURCE

#include <errno.h>
#include <error.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define NAME "timemachine"
#define ENV "TIME_MACHINE_OFFSET"
#define GET "get"
#define SET "set"


static time_t offset = 0;
static pthread_t thread;


static void read_offset() {
    FILE* file = fopen(SET, "r");
    if (file == NULL) {
        error(0, errno, "%s: error opening file '%s'", NAME, SET);
        return;
    }
    if (fscanf(file, "%ld\n", &offset) <= 0) {
        error(0, errno, "%s: error reading file '%s'", NAME, SET);
    }
    if (fclose(file) == EOF) {
        error(0, errno, "%s: error closing file '%s'", NAME, SET);
    }
}

static void write_offset() {
    FILE* file = fopen(GET,"w+");
    if (file == NULL) {
        error(0, errno, "%s: error opening file '%s'", NAME, GET);
        return;
    }
    if (fprintf(file, "%ld\n", offset) <= 0) {
        error(0, errno, "%s: error writing file '%s'", NAME, GET);
    }
    if (fclose(file) == EOF) {
        error(0, errno, "%s: error closing file '%s'", NAME, GET);
    }
}

static void* run(__attribute__((unused)) void* arg) {
    while(1) {
        read_offset();
        write_offset();
    }
    return NULL;
}

__attribute__((constructor (101)))
static void env() {
    char* text = getenv(ENV);
    if (text == NULL) {
        return;
    }
    if (sscanf(text, "%ld\n", &offset) <= 0) {
        error(0, errno, "%s: error parsing value '%s'", NAME, text);
    }
}

__attribute__((constructor (102)))
static void create_dir() {
}

__attribute__((constructor (103)))
static void create_set() {
    if (mkfifo(SET, 0660) < 0) {
        error(0, errno, "%s: error creating file '%s'", NAME, SET);
    }
}

__attribute__((constructor (104)))
static void create_get() {
    write_offset();
}

__attribute__((constructor (110)))
static void start() {
    pthread_create(&thread, NULL, run, NULL);
}

__attribute__((destructor (110)))
static void stop() {
}


typedef time_t (*func_time_t)(time_t*);

time_t time(time_t *tloc) {
    static func_time_t func;
    if (func == NULL) {
        func = (func_time_t) dlsym(RTLD_NEXT, "time");
    }
    time_t res = func(tloc) + offset;
    if (tloc != NULL) {
        *tloc += offset;
    }
    return res;
}

typedef int (*func_clock_gettime_t)(clockid_t, struct timespec*);

int clock_gettime(clockid_t clk_id, struct timespec *res) {
    static func_clock_gettime_t func;
    if (func == NULL) {
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
    if (func == NULL) {
        func = (func_gettimeofday_t) dlsym(RTLD_NEXT, "gettimeofday");
    }
    int err = func(tv, tz);
    tv->tv_sec += offset;
    return err;
}

