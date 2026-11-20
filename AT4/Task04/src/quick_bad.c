#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static long long comparisons = 0;
static long long swaps = 0;
static int max_depth = 0;

static inline void swap(int *a, int *b)
{
    int t = *a; *a = *b; *b = t;
}

static void quick(int a[], int lo, int hi, int depth)
{
    if (depth > max_depth) max_depth = depth;
    if (lo >= hi) return;

    int p = a[hi];
    int i = lo - 1;

    for (int j = lo; j < hi; j++) {
        comparisons++;
        if (a[j] <= p) {
            i++;
            swaps++;
            swap(&a[i], &a[j]);
        }
    }
    swaps++;
    swap(&a[i + 1], &a[hi]);

    quick(a, lo, i, depth + 1);
    quick(a, i + 2, hi, depth + 1);
}

static double get_time_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

int main(void)
{
    int n = 10000;
    int *arr_random = malloc(n * sizeof(int));
    int *arr_sorted = malloc(n * sizeof(int));

    srand(42);
    for (int i = 0; i < n; i++) {
        arr_random[i] = rand() % 100000;
        arr_sorted[i] = i + 1;
    }

    FILE *out = fopen("Task04/output/quick_bad_results.txt", "w");
    if (!out) {
        perror("Failed to open quick_bad_results.txt");
        free(arr_random);
        free(arr_sorted);
        return 1;
    }

    fprintf(out, "================================================================================\n");
    fprintf(out, "           FORENSIC ANALYSIS: BAD QUICKSORT (Last Element Pivot)\n");
    fprintf(out, "================================================================================\n\n");

    /* 1. Benchmark on 10,000 Random */
    comparisons = 0; swaps = 0; max_depth = 0;
    double t0 = get_time_sec();
    quick(arr_random, 0, n - 1, 1);
    double t1 = get_time_sec();
    double time_random = (t1 - t0) * 1000.0;

    fprintf(out, "1. Random Input (N = %d):\n", n);
    fprintf(out, "   - Time:              %.2f ms\n", time_random);
    fprintf(out, "   - Comparisons:       %lld (~%.2f * N log2 N)\n", comparisons, (double)comparisons / (n * 13.2877));
    fprintf(out, "   - Swaps:             %lld\n", swaps);
    fprintf(out, "   - Max Stack Depth:   %d\n\n", max_depth);

    /* 2. Benchmark on 10,000 Sorted */
    comparisons = 0; swaps = 0; max_depth = 0;
    t0 = get_time_sec();
    quick(arr_sorted, 0, n - 1, 1);
    t1 = get_time_sec();
    double time_sorted = (t1 - t0) * 1000.0;

    fprintf(out, "2. Already Sorted Input (N = %d) - PATHOLOGICAL CASE:\n", n);
    fprintf(out, "   - Time:              %.2f ms (%.1fx slowdown!)\n", time_sorted, time_sorted / (time_random > 0.01 ? time_random : 0.01));
    fprintf(out, "   - Comparisons:       %lld (Exact N*(N-1)/2 = %lld)\n", comparisons, (long long)n * (n - 1) / 2);
    fprintf(out, "   - Swaps:             %lld\n", swaps);
    fprintf(out, "   - Max Stack Depth:   %d (Degenerates to O(N) recursion depth!)\n\n", max_depth);

    fprintf(out, "Forensic Diagnosis:\n");
    fprintf(out, "When the input is already sorted, choosing the last element a[hi] as pivot always selects\n");
    fprintf(out, "the maximum element of the partition. The partition splits into (N-1) and 0 elements,\n");
    fprintf(out, "collapsing the recursion tree into a degenerate linked list of depth N = 10,000.\n");
    fprintf(out, "Comparisons explode to N*(N-1)/2 = 49,995,000, causing severe performance degradation\n");
    fprintf(out, "and catastrophic risk of call stack overflow.\n");

    fclose(out);
    free(arr_random);
    free(arr_sorted);

    printf("Bad Quicksort forensic benchmark complete. Saved to Task04/output/quick_bad_results.txt\n");
    return 0;
}
