#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <time.h>

#define TOTAL_ITEMS 10000000
#define CUTOFF 65536
#define NUM_RUNS 5

static void merge(int a[], int tmp[], int lo, int mid, int hi)
{
    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi) {
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

static void merge_sort_seq(int a[], int tmp[], int lo, int hi)
{
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort_seq(a, tmp, lo, mid);
    merge_sort_seq(a, tmp, mid + 1, hi);
    merge(a, tmp, lo, mid, hi);
}

static void merge_sort_parallel(int a[], int tmp[], int lo, int hi)
{
    if (lo >= hi) return;
    if (hi - lo + 1 <= CUTOFF) {
        merge_sort_seq(a, tmp, lo, hi);
        return;
    }

    int mid = lo + (hi - lo) / 2;

    #pragma omp task shared(a, tmp)
    merge_sort_parallel(a, tmp, lo, mid);

    #pragma omp task shared(a, tmp)
    merge_sort_parallel(a, tmp, mid + 1, hi);

    #pragma omp taskwait
    merge(a, tmp, lo, mid, hi);
}

static void run_parallel_sort(int a[], int tmp[], int n, int threads)
{
    omp_set_num_threads(threads);
    #pragma omp parallel
    {
        #pragma omp single nowait
        {
            merge_sort_parallel(a, tmp, 0, n - 1);
        }
    }
}

static double get_time_sec(void)
{
    return omp_get_wtime();
}

int main(void)
{
    int thread_counts[] = {1, 2, 4, 8};
    int n_threads = 4;

    printf("=========================================================================================================\n");
    printf("           TASK 5: PARALLEL MERGE SORT BENCHMARK (N = %d elements, %d runs)\n", TOTAL_ITEMS, NUM_RUNS);
    printf("=========================================================================================================\n");
    printf("Machine Logical Cores Detected: %d\n", omp_get_num_procs());
    printf("Cutoff threshold for sequential leaf tasks: %d items\n\n", CUTOFF);

    int *master_data = malloc(TOTAL_ITEMS * sizeof(int));
    int *test_data = malloc(TOTAL_ITEMS * sizeof(int));
    int *tmp_buffer = malloc(TOTAL_ITEMS * sizeof(int));

    if (!master_data || !test_data || !tmp_buffer) {
        perror("Failed to allocate memory for 10M integers");
        return 1;
    }

    srand(2026);
    for (int i = 0; i < TOTAL_ITEMS; i++) {
        master_data[i] = rand();
    }

    FILE *csv = fopen("Task05/output/parallel_speedup.csv", "w");
    if (!csv) {
        perror("Failed to open parallel_speedup.csv");
        return 1;
    }
    fprintf(csv, "Threads,AvgTimeSec,Speedup,EfficiencyPercent\n");

    printf("%-8s | %-16s | %-14s | %-18s\n", "Threads", "Avg Time (sec)", "Speedup (T1/Tp)", "Efficiency (%)");
    printf("---------------------------------------------------------------------------------------------------------\n");

    double t1_time = 0.0;

    for (int t = 0; t < n_threads; t++) {
        int threads = thread_counts[t];
        double total_time = 0.0;

        for (int r = 0; r < NUM_RUNS; r++) {
            memcpy(test_data, master_data, TOTAL_ITEMS * sizeof(int));

            double start = get_time_sec();
            run_parallel_sort(test_data, tmp_buffer, TOTAL_ITEMS, threads);
            double end = get_time_sec();

            total_time += (end - start);

            /* Verify correctness on first run */
            if (r == 0) {
                for (int i = 0; i < TOTAL_ITEMS - 1; i++) {
                    if (test_data[i] > test_data[i + 1]) {
                        fprintf(stderr, "Sorting verification failed for threads=%d!\n", threads);
                        break;
                    }
                }
            }
        }

        double avg_time = total_time / (double)NUM_RUNS;
        if (threads == 1) {
            t1_time = avg_time;
        }

        double speedup = t1_time / avg_time;
        double efficiency = (speedup / (double)threads) * 100.0;

        printf("%-8d | %16.3f | %14.2fx | %17.1f%%\n", threads, avg_time, speedup, efficiency);
        fprintf(csv, "%d,%.4f,%.2f,%.2f\n", threads, avg_time, speedup, efficiency);
    }
    printf("=========================================================================================================\n");

    fclose(csv);
    free(master_data);
    free(test_data);
    free(tmp_buffer);

    printf("Parallel merge sort benchmark complete. Saved to Task05/output/parallel_speedup.csv\n");
    return 0;
}
