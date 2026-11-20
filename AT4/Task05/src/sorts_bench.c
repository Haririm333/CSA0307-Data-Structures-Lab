#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double time_ms;
    long long comparisons;
    bool skipped;
    bool estimated;
} SortResult;

static inline void swap_int(int *a, int *b)
{
    int t = *a; *a = *b; *b = t;
}

static double get_time_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* 1. Bubble Sort */
static SortResult run_bubble(int a[], int n)
{
    SortResult r = {0.0, 0, false, false};
    double t0 = get_time_sec();
    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - i - 1; j++) {
            r.comparisons++;
            if (a[j] > a[j + 1]) {
                swap_int(&a[j], &a[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
    double t1 = get_time_sec();
    r.time_ms = (t1 - t0) * 1000.0;
    return r;
}

/* 2. Insertion Sort */
static SortResult run_insertion(int a[], int n)
{
    SortResult r = {0.0, 0, false, false};
    double t0 = get_time_sec();
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0) {
            r.comparisons++;
            if (a[j] > key) {
                a[j + 1] = a[j];
                j--;
            } else {
                break;
            }
        }
        a[j + 1] = key;
    }
    double t1 = get_time_sec();
    r.time_ms = (t1 - t0) * 1000.0;
    return r;
}

/* 3. Selection Sort */
static SortResult run_selection(int a[], int n)
{
    SortResult r = {0.0, 0, false, false};
    double t0 = get_time_sec();
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            r.comparisons++;
            if (a[j] < a[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            swap_int(&a[i], &a[min_idx]);
        }
    }
    double t1 = get_time_sec();
    r.time_ms = (t1 - t0) * 1000.0;
    return r;
}

/* 4. Merge Sort */
static void merge_impl(int a[], int tmp[], int lo, int mid, int hi, long long *comps)
{
    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi) {
        (*comps)++;
        if (a[i] <= a[j]) {
            tmp[k++] = a[i++];
        } else {
            tmp[k++] = a[j++];
        }
    }
    while (i <= mid) tmp[k++] = a[i++];
    while (j <= hi)  tmp[k++] = a[j++];
    for (int p = lo; p <= hi; p++) a[p] = tmp[p];
}

static void merge_sort_rec(int a[], int tmp[], int lo, int hi, long long *comps)
{
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort_rec(a, tmp, lo, mid, comps);
    merge_sort_rec(a, tmp, mid + 1, hi, comps);
    merge_impl(a, tmp, lo, mid, hi, comps);
}

static SortResult run_merge(int a[], int n)
{
    SortResult r = {0.0, 0, false, false};
    int *tmp = malloc(n * sizeof(int));
    double t0 = get_time_sec();
    merge_sort_rec(a, tmp, 0, n - 1, &r.comparisons);
    double t1 = get_time_sec();
    r.time_ms = (t1 - t0) * 1000.0;
    free(tmp);
    return r;
}

/* 5. Quick Sort (Median-of-three) */
static void quick_m3_rec(int a[], int lo, int hi, long long *comps)
{
    while (lo < hi) {
        if (hi - lo < 10) {
            for (int i = lo + 1; i <= hi; i++) {
                int key = a[i];
                int j = i - 1;
                while (j >= lo) {
                    (*comps)++;
                    if (a[j] > key) {
                        a[j + 1] = a[j];
                        j--;
                    } else break;
                }
                a[j + 1] = key;
            }
            return;
        }

        int mid = lo + (hi - lo) / 2;
        (*comps)++; if (a[lo] > a[mid]) swap_int(&a[lo], &a[mid]);
        (*comps)++; if (a[lo] > a[hi])  swap_int(&a[lo], &a[hi]);
        (*comps)++; if (a[mid] > a[hi]) swap_int(&a[mid], &a[hi]);
        swap_int(&a[mid], &a[hi - 1]);
        int pivot = a[hi - 1];

        int i = lo, j = hi - 1;
        for (;;) {
            while (1) { (*comps)++; if (a[++i] < pivot) { } else break; }
            while (1) { (*comps)++; if (a[--j] > pivot) { } else break; }
            if (i >= j) break;
            swap_int(&a[i], &a[j]);
        }
        swap_int(&a[i], &a[hi - 1]);

        if (i - lo < hi - i) {
            quick_m3_rec(a, lo, i - 1, comps);
            lo = i + 1;
        } else {
            quick_m3_rec(a, i + 1, hi, comps);
            hi = i - 1;
        }
    }
}

static SortResult run_quick(int a[], int n)
{
    SortResult r = {0.0, 0, false, false};
    double t0 = get_time_sec();
    quick_m3_rec(a, 0, n - 1, &r.comparisons);
    double t1 = get_time_sec();
    r.time_ms = (t1 - t0) * 1000.0;
    return r;
}

/* 6. Heap Sort */
static void heap_sift_down(int a[], int n, int i, long long *comps)
{
    while (2 * i + 1 < n) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int largest = i;

        (*comps)++;
        if (a[left] > a[largest]) largest = left;
        if (right < n) {
            (*comps)++;
            if (a[right] > a[largest]) largest = right;
        }
        if (largest == i) break;
        swap_int(&a[i], &a[largest]);
        i = largest;
    }
}

static SortResult run_heap(int a[], int n)
{
    SortResult r = {0.0, 0, false, false};
    double t0 = get_time_sec();
    for (int i = n / 2 - 1; i >= 0; i--) {
        heap_sift_down(a, n, i, &r.comparisons);
    }
    for (int i = n - 1; i > 0; i--) {
        swap_int(&a[0], &a[i]);
        heap_sift_down(a, i, 0, &r.comparisons);
    }
    double t1 = get_time_sec();
    r.time_ms = (t1 - t0) * 1000.0;
    return r;
}

int main(void)
{
    int sizes[] = {1000, 100000, 1000000};
    int n_sizes = 3;
    const char *orders[] = {"Random", "Sorted", "Reverse"};
    int n_orders = 3;

    typedef struct {
        const char *name;
        bool is_quadratic;
        const char *stability;
        const char *extra_space;
        SortResult (*fn)(int a[], int n);
    } AlgInfo;

    AlgInfo algs[] = {
        {"Bubble Sort",    true,  "Stable",   "O(1)", run_bubble},
        {"Insertion Sort", true,  "Stable",   "O(1)", run_insertion},
        {"Selection Sort", true,  "Unstable", "O(1)", run_selection},
        {"Merge Sort",     false, "Stable",   "O(n)", run_merge},
        {"Quick Sort(M3)", false, "Unstable", "O(log n)", run_quick},
        {"Heap Sort",      false, "Unstable", "O(1)", run_heap}
    };
    int n_algs = sizeof(algs) / sizeof(algs[0]);

    FILE *csv = fopen("Task05/output/sorts_bench.csv", "w");
    if (!csv) {
        perror("Failed to open sorts_bench.csv");
        return 1;
    }

    fprintf(csv, "Algorithm,Order,N,TimeMs,Comparisons,Stability,ExtraSpace,Status\n");

    printf("==================================================================================================================\n");
    printf("%-18s %-9s %-9s %-14s %-16s %-10s %-12s %-14s\n",
           "Algorithm", "Order", "N", "Time (ms)", "Comparisons", "Stability", "Extra Space", "Status");
    printf("==================================================================================================================\n");

    srand(555);

    /* Store 100K results for O(n^2) extrapolation */
    double ext_time[6][3] = {{0}};
    long long ext_comps[6][3] = {{0}};

    for (int s = 0; s < n_sizes; s++) {
        int n = sizes[s];

        for (int o = 0; o < n_orders; o++) {
            const char *order = orders[o];

            /* Generate baseline data */
            int *base = malloc(n * sizeof(int));
            if (strcmp(order, "Random") == 0) {
                for (int i = 0; i < n; i++) base[i] = rand() % (n * 10);
            } else if (strcmp(order, "Sorted") == 0) {
                for (int i = 0; i < n; i++) base[i] = i + 1;
            } else {
                for (int i = 0; i < n; i++) base[i] = n - i;
            }

            for (int a = 0; a < n_algs; a++) {
                /* Check if O(n^2) should be skipped at 1,000,000 */
                if (n == 1000000 && algs[a].is_quadratic) {
                    /* Special case: Insertion Sort on Sorted is O(n), can run */
                    if (strcmp(algs[a].name, "Insertion Sort") == 0 && strcmp(order, "Sorted") == 0) {
                        int *arr = malloc(n * sizeof(int));
                        memcpy(arr, base, n * sizeof(int));
                        SortResult res = algs[a].fn(arr, n);
                        free(arr);
                        printf("%-18s %-9s %-9d %14.2f %16lld %-10s %-12s %-14s\n",
                               algs[a].name, order, n, res.time_ms, res.comparisons,
                               algs[a].stability, algs[a].extra_space, "Measured O(n)");
                        fprintf(csv, "\"%s\",%s,%d,%.2f,%lld,%s,%s,Measured\n",
                                algs[a].name, order, n, res.time_ms, res.comparisons,
                                algs[a].stability, algs[a].extra_space);
                        continue;
                    }

                    /* Extrapolate from 100,000: N grows 10x, N^2 grows 100x */
                    double est_t = ext_time[a][o] * 100.0;
                    long long est_c = ext_comps[a][o] * 100LL;
                    printf("%-18s %-9s %-9d %14.2f %16lld %-10s %-12s %-14s\n",
                           algs[a].name, order, n, est_t, est_c,
                           algs[a].stability, algs[a].extra_space, "skipped (est.)");
                    fprintf(csv, "\"%s\",%s,%d,%.2f,%lld,%s,%s,skipped (est.)\n",
                            algs[a].name, order, n, est_t, est_c,
                            algs[a].stability, algs[a].extra_space);
                    continue;
                }

                int *arr = malloc(n * sizeof(int));
                memcpy(arr, base, n * sizeof(int));
                SortResult res = algs[a].fn(arr, n);
                free(arr);

                if (n == 100000) {
                    ext_time[a][o] = res.time_ms;
                    ext_comps[a][o] = res.comparisons;
                }

                printf("%-18s %-9s %-9d %14.2f %16lld %-10s %-12s %-14s\n",
                       algs[a].name, order, n, res.time_ms, res.comparisons,
                       algs[a].stability, algs[a].extra_space, "Measured");
                fprintf(csv, "\"%s\",%s,%d,%.2f,%lld,%s,%s,Measured\n",
                        algs[a].name, order, n, res.time_ms, res.comparisons,
                        algs[a].stability, algs[a].extra_space);
            }
            free(base);
        }
    }
    printf("==================================================================================================================\n");

    fclose(csv);
    printf("Sorting benchmark complete. Saved to Task05/output/sorts_bench.csv\n");
    return 0;
}
