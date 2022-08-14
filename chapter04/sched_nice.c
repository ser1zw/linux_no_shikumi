#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NLOOP_FOR_ESTIMATION 1000000000UL
#define NSECS_PER_MSEC 1000000UL
#define NSECS_PER_SEC 1000000000UL

static inline long diff_nsec(struct timespec before, struct timespec after) {
    return ((after.tv_sec * NSECS_PER_SEC + after.tv_nsec)
            - (before.tv_sec * NSECS_PER_SEC + before.tv_nsec));
}

static inline void load(unsigned long nloop) {
    volatile unsigned long n = 0;
    for (unsigned long i = 0; i < nloop; i++) {
        n++;
    }
}

unsigned long loops_per_msec() {
    struct timespec before, after;

    clock_gettime(CLOCK_MONOTONIC, &before);
    load(NLOOP_FOR_ESTIMATION);
    clock_gettime(CLOCK_MONOTONIC, &after);

    return NLOOP_FOR_ESTIMATION / (diff_nsec(before, after) / NSECS_PER_MSEC);
}

void kill_pids(pid_t* pids, size_t ncreated) {
    for (int i = 0; i < ncreated; i++) {
        int ret = kill(pids[i], SIGKILL);
        if (ret < 0) {
            fprintf(stderr, "Failed to kill process: %d\n", pids[i]);
        }
    }
}

void child(int id, struct timespec *buf, int nrecord, unsigned long nloop_per_resolution, struct timespec start) {
    for (int i = 0; i < nrecord; i++) {
        struct timespec ts;
        load(nloop_per_resolution);
        clock_gettime(CLOCK_MONOTONIC, &ts);
        buf[i] = ts;
    }
    for (int i = 0; i < nrecord; i++) {
        long diff_msec = diff_nsec(start, buf[i]) / NSECS_PER_MSEC;
        int progress = (i + 1) * 100 / nrecord;
        printf("%d\t%ld\t%d\n", id, diff_msec, progress);
    }
    exit(EXIT_SUCCESS);
}

void parent(pid_t *pids, int ncreated) {
    for (int i = 0; i < ncreated; i++) {
        int ret = wait(NULL);
        if (ret < 0) {
            fprintf(stderr, "Failed to wait child process %d\n", pids[i]);
        }
    }
}

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <total[ms]> <resolution[ms]>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int nproc = 2;
    int total = atoi(argv[1]);
    int resolution = atoi(argv[2]);

    if (total < 1) {
        fprintf(stderr, "<total[ms]> should be >= 1\n");
        exit(EXIT_FAILURE);
    }
    if (resolution < 1) {
        fprintf(stderr, "<resolution[ms]> should be >= 1\n");
        exit(EXIT_FAILURE);
    }
    if ((total % resolution) != 0) {
        fprintf(stderr, "<total>(%d) should be multiple of <resolution[ms]>(%d)\n", total, resolution);
        exit(EXIT_FAILURE);
    }

    int nrecord = total / resolution;
    struct timespec *logbuf;
    logbuf = (struct timespec*)malloc(sizeof(struct timespec) * nrecord);
    if (logbuf == NULL) {
        fprintf(stderr, "Could not allocate memory for `logbuf`\n");
        exit(EXIT_FAILURE);
    }

    puts("Estimating workload which takes just one milisecond");
    unsigned long nloop_per_resolution = loops_per_msec() * resolution;
    printf("nloop_per_resolution = %ld\n", nloop_per_resolution);
    puts("Estimation finished");
    printf("Process#\tElapsed(msec)\tProgress(%%)\n");
    fflush(stdout);

    pid_t *pids;
    pids = (pid_t*)malloc(sizeof(pid_t) * nproc);
    if (pids == NULL) {
        fprintf(stderr, "Could not allocate memory for `pids`\n");
        free(logbuf);
        exit(EXIT_FAILURE);
    }

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    size_t ncreated = 0;
    for (int i = 0; i < nproc; i++) {
        pids[i] = fork();
        ncreated++;
        if (pids[i] == -1) {
            kill_pids(pids, ncreated);
            break;
        } else if (pids[i] == 0) {
            if (i == 1) {
                nice(5);
            }
            child(i, logbuf, nrecord, nloop_per_resolution, start);
            abort(); // Never reach here
        }
    }

    parent(pids, ncreated);
    free(pids);
    free(logbuf);

    return 0;
}
