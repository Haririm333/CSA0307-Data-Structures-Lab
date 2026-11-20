#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

typedef struct {
    long long comparisons;
    long long swaps;
    double time_ms;
} BenchStats;

static double get_time_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* Sift-down for bottom-up Build-Heap */
static void sift_down(int a[], int n, int i, long long *comps, long long *swaps)
{
    while (2 * i + 1 < n) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int largest = i;

        (*comps)++;
        if (a[left] > a[largest]) {
            largest = left;
        }

        if (right < n) {
            (*comps)++;
            if (a[right] > a[largest]) {
                largest = right;
            }
        }

        if (largest == i) break;

        int t = a[i]; a[i] = a[largest]; a[largest] = t;
        (*swaps)++;
        i = largest;
    }
}

/* Bottom-up Build-Heap: O(n) */
static BenchStats run_build_heap(const int orig[], int n)
{
    int *a = malloc(n * sizeof(int));
    memcpy(a, orig, n * sizeof(int));

    BenchStats s = {0, 0, 0.0};
    double t0 = get_time_sec();

    for (int i = n / 2 - 1; i >= 0; i--) {
        sift_down(a, n, i, &s.comparisons, &s.swaps);
    }

    double t1 = get_time_sec();
    s.time_ms = (t1 - t0) * 1000.0;
    free(a);
    return s;
}

/* Sift-up for successive insertions: O(n log n) */
static void sift_up(int a[], int i, long long *comps, long long *swaps)
{
    while (i > 0) {
        int parent = (i - 1) / 2;
        (*comps)++;
        if (a[i] > a[parent]) {
            int t = a[i]; a[i] = a[parent]; a[parent] = t;
            (*swaps)++;
            i = parent;
        } else {
            break;
        }
    }
}

/* Successive Insertions */
static BenchStats run_successive_inserts(const int orig[], int n)
{
    int *a = malloc(n * sizeof(int));
    BenchStats s = {0, 0, 0.0};
    double t0 = get_time_sec();

    for (int i = 0; i < n; i++) {
        a[i] = orig[i];
        sift_up(a, i, &s.comparisons, &s.swaps);
    }

    double t1 = get_time_sec();
    s.time_ms = (t1 - t0) * 1000.0;
    free(a);
    return s;
}

int main(void)
{
    int sizes[] = {1000, 10000, 100000, 1000000};
    int n_sizes = (int)(sizeof(sizes) / sizeof(sizes[0]));

    FILE *csv = fopen("Task03/output/buildheap_bench.csv", "w");
    if (!csv) {
        perror("Failed to open buildheap_bench.csv");
        return 1;
    }

    fprintf(csv, "N,BuildHeap_Comps,BuildHeap_Swaps,BuildHeap_RatioCompsN,BuildHeap_TimeMs,"
                 "Successive_Comps,Successive_Swaps,Successive_RatioCompsN,Successive_TimeMs\n");

    printf("========================================================================================================================\n");
    printf("                                   BUILD-HEAP (O(n)) vs SUCCESSIVE INSERTS (O(n log n))\n");
    printf("========================================================================================================================\n");
    printf("%-9s | %-16s %-14s %-9s %-9s | %-16s %-14s %-9s %-9s\n",
           "N", "BuildHeap Comps", "Swaps", "Comps/N", "Time(ms)", "Successive Comps", "Swaps", "Comps/N", "Time(ms)");
    printf("------------------------------------------------------------------------------------------------------------------------\n");

    srand(2026);

    for (int k = 0; k < n_sizes; k++) {
        int n = sizes[k];
        int *data = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) {
            data[i] = rand();
        }

        BenchStats s_build = run_build_heap(data, n);
        BenchStats s_succ = run_successive_inserts(data, n);

        double ratio_build = (double)s_build.comparisons / n;
        double ratio_succ = (double)s_succ.comparisons / n;

        printf("%-9d | %-16lld %-14lld %-9.2f %-9.2f | %-16lld %-14lld %-9.2f %-9.2f\n",
               n, s_build.comparisons, s_build.swaps, ratio_build, s_build.time_ms,
               s_succ.comparisons, s_succ.swaps, ratio_succ, s_succ.time_ms);

        fprintf(csv, "%d,%lld,%lld,%.4f,%.4f,%lld,%lld,%.4f,%.4f\n",
                n, s_build.comparisons, s_build.swaps, ratio_build, s_build.time_ms,
                s_succ.comparisons, s_succ.swaps, ratio_succ, s_succ.time_ms);

        free(data);
    }
    printf("========================================================================================================================\n");

    fclose(csv);
    printf("Benchmark results saved to Task03/output/buildheap_bench.csv\n");
    return 0;
}
