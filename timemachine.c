#define _GNU_SOURCE

#include <fcntl.h>
#include <errno.h>
#include <error.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#define NAME "timemachine"
#define BASE "/tmp/timemachine"
#define ENV "TIME_MACHINE_OFFSET"
#define GET "get"
#define SET "set"

static pid_t pid;
static char* dir;
static char* get;
static char* set;
static pthread_t thread;
static time_t offset = 0;

static void write_offset() {
    FILE* file = fopen(get, "w+");
    if (file == NULL) {
        error(0, errno, "%s: error opening file '%s'", NAME, get);
        return;
    }
    if (fprintf(file, "%ld\n", offset) <= 0) {
        error(0, errno, "%s: error writing file '%s'", NAME, get);
    }
    if (fclose(file) == EOF) {
        error(0, errno, "%s: error closing file '%s'", NAME, get);
    }
}

static void read_offset() {
    FILE* file = fopen(set, "r");
    if (file == NULL) {
        error(0, errno, "%s: error opening file '%s'", NAME, set);
        return;
    }
    if (fscanf(file, "%ld\n", &offset) <= 0) {
        error(0, errno, "%s: error reading file '%s'", NAME, set);
    }
    if (fclose(file) == EOF) {
        error(0, errno, "%s: error closing file '%s'", NAME, set);
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
static void values() {
    pid = getpid();
    asprintf(&dir, "%s/%d", BASE, pid);
    asprintf(&get, "%s/%s", dir, GET);
    asprintf(&set, "%s/%s", dir, SET);
}

__attribute__((constructor (102)))
static void env() {
    char* text = getenv(ENV);
    if (text == NULL) {
        return;
    }
    if (sscanf(text, "%ld\n", &offset) <= 0) {
        error(0, errno, "%s: error parsing value '%s'", NAME, text);
    }
}

__attribute__((constructor (103)))
static void create_base() {
    if (mkdir(BASE, 0777) < 0) {
        if (errno == EEXIST) {
            return;
        }
        error(0, errno, "%s: error creating base directory '%s'", NAME, BASE);
    }
}

__attribute__((constructor (104)))
static void create_dir() {
    if (mkdir(dir, 0755) < 0) {
        error(0, errno, "%s: error creating process directory '%s'", NAME, dir);
    }
}

__attribute__((constructor (105)))
static void create_get() {
    write_offset();
}

__attribute__((constructor (106)))
static void create_set() {
    if (mkfifo(set, 0660) < 0) {
        error(0, errno, "%s: error creating file '%s'", NAME, set);
    }
}

__attribute__((constructor (110)))
static void start() {
    pthread_create(&thread, NULL, run, NULL);
}


__attribute__((destructor (103)))
static void remove_set() {
    if (unlink(set) < 0) {
        error(0, errno, "%s: error removing file '%s'", NAME, set);
    }
}

__attribute__((destructor (102)))
static void remove_get() {
    if (unlink(get) < 0) {
        error(0, errno, "%s: error removing file '%s'", NAME, get);
    }
}

__attribute__((destructor (101)))
static void remove_dir() {
    if (rmdir(dir) < 0) {
        error(0, errno, "%s: error removing process directory '%s'", NAME, dir);
    }
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

