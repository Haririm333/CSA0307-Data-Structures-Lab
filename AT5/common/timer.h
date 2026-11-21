#ifndef TIMER_H
#define TIMER_H

#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include <stdint.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#else
#include <sys/resource.h>
#endif

/* Return current monotonic time in seconds */
static inline double get_time_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* Return peak RSS in kilobytes */
static inline long get_peak_rss_kb(void) {
#if defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return (long)(pmc.PeakWorkingSetSize / 1024);
    }
    return 0;
#else
    struct rusage r;
    if (getrusage(RUSAGE_SELF, &r) == 0) {
        return (long)r.ru_maxrss;
    }
    return 0;
#endif
}

/* Helper to compare doubles for qsort */
static inline int compare_doubles(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

/* Median of 3 timing runs */
static inline double run_median_3(double (*func)(void *arg), void *arg) {
    double times[3];
    for (int i = 0; i < 3; i++) {
        times[i] = func(arg);
    }
    qsort(times, 3, sizeof(double), compare_doubles);
    return times[1];
}

#endif /* TIMER_H */
