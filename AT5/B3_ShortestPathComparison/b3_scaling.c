#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "sp_lib.h"

#define NUM_SOURCES 10

/* Benchmarking helper for single-source Dijkstra averaged over 10 sources */
static double bench_dijkstra_10_sources(const CSRGraph *g, int *dist, bool is_heap) {
    int sources[NUM_SOURCES];
    srand(12345);
    for (int i = 0; i < NUM_SOURCES; i++) {
        sources[i] = rand() % g->n;
    }

    double t0 = get_time_sec();
    for (int i = 0; i < NUM_SOURCES; i++) {
        if (is_heap) {
            dijkstra_heap_single(g, sources[i], dist);
        } else {
            dijkstra_array_single(g, sources[i], dist);
        }
    }
    double t1 = get_time_sec();
    return (t1 - t0) / (double)NUM_SOURCES;
}

int main(void) {
    printf("========================================================================================================\n");
    printf("        B3.2: SHORTEST PATH SCALING BENCHMARK (V = 100, 1,000, 10,000; AVG DEGREE ~3)                   \n");
    printf("========================================================================================================\n\n");

    int test_v[] = {100, 1000, 10000};
    int num_tests = 3;

    double fw_times[3] = {0.0, 0.0, 0.0};
    double dij_arr_times[3] = {0.0, 0.0, 0.0};
    double dij_heap_times[3] = {0.0, 0.0, 0.0};

    size_t csr_analytic_mem[3];
    size_t heap_extra_mem[3];
    long peak_rss[3];

    FILE *csv = fopen("results.csv", "w");
    if (csv) {
        fprintf(csv, "V,E,Dijkstra_Arr_Time_s,Dijkstra_Heap_Time_s,FloydWarshall_Time_s,Is_FW_Estimated,CSR_Mem_Bytes,Heap_Mem_Bytes,Peak_RSS_KB\n");
    }

    for (int t = 0; t < num_tests; t++) {
        int n = test_v[t];
        unsigned int seed = 42 + (unsigned int)t;
        CSRGraph *g = generate_connected_graph_path(n, 3, 1, 100, seed);
        if (!g) continue;

        csr_analytic_mem[t] = csr_analytic_bytes(g->n, g->m, true);
        heap_extra_mem[t] = sizeof(MinHeap) + (size_t)(2 * n) * sizeof(HeapNode);

        int *dist = (int *)malloc(sizeof(int) * (size_t)n);

        /* Measure Dijkstra Array (average of 10 sources, median of 3 runs) */
        double arr_runs[3];
        for (int r = 0; r < 3; r++) arr_runs[r] = bench_dijkstra_10_sources(g, dist, false);
        qsort(arr_runs, 3, sizeof(double), compare_doubles);
        dij_arr_times[t] = arr_runs[1];

        /* Measure Dijkstra Heap (average of 10 sources, median of 3 runs) */
        double heap_runs[3];
        for (int r = 0; r < 3; r++) heap_runs[r] = bench_dijkstra_10_sources(g, dist, true);
        qsort(heap_runs, 3, sizeof(double), compare_doubles);
        dij_heap_times[t] = heap_runs[1];

        /* Measure Floyd-Warshall */
        if (n <= 1000) {
            int *mat = (int *)malloc(sizeof(int) * (size_t)(n * n));
            csr_to_flat_matrix(g, mat);
            int *fw_out = (int *)malloc(sizeof(int) * (size_t)(n * n));

            double fw_runs[3];
            for (int r = 0; r < 3; r++) {
                double t0 = get_time_sec();
                floyd_warshall_flat(n, mat, fw_out);
                double t1 = get_time_sec();
                fw_runs[r] = t1 - t0;
            }
            qsort(fw_runs, 3, sizeof(double), compare_doubles);
            fw_times[t] = fw_runs[1];

            free(mat);
            free(fw_out);
        } else {
            /* V = 10,000: Estimated via cubic scaling */
            /* Theoretical growth: (10,000 / 1,000)^3 = 10^3 = 1,000x */
            fw_times[t] = fw_times[1] * 1000.0;
        }

        peak_rss[t] = get_peak_rss_kb();

        free(dist);
        csr_free(g);
    }

    /* Print Table */
    printf("%-7s | %-16s | %-16s | %-24s | %-12s | %-10s\n",
           "V", "Dijkstra Arr (s)", "Dijkstra Heap(s)", "Floyd-Warshall (s)", "CSR Analytic", "Peak RSS");
    printf("--------------------------------------------------------------------------------------------------------\n");

    for (int t = 0; t < num_tests; t++) {
        char fw_str[48];
        if (t == 2) {
            snprintf(fw_str, sizeof(fw_str), "%.3f s [ESTIMATED]", fw_times[t]);
        } else {
            snprintf(fw_str, sizeof(fw_str), "%.6f s", fw_times[t]);
        }

        char mem_str[32];
        if (csr_analytic_mem[t] >= 1024 * 1024) {
            snprintf(mem_str, sizeof(mem_str), "%.2f MB", (double)csr_analytic_mem[t] / (1024.0 * 1024.0));
        } else {
            snprintf(mem_str, sizeof(mem_str), "%.2f KB", (double)csr_analytic_mem[t] / 1024.0);
        }

        printf("%-7d | %-16.6f | %-16.6f | %-24s | %-12s | %ld KB\n",
               test_v[t], dij_arr_times[t], dij_heap_times[t], fw_str, mem_str, peak_rss[t]);

        if (csv) {
            fprintf(csv, "%d,%d,%.6f,%.6f,%.6f,%s,%zu,%zu,%ld\n",
                    test_v[t], test_v[t] * 3, dij_arr_times[t], dij_heap_times[t],
                    fw_times[t], (t == 2) ? "YES" : "NO",
                    csr_analytic_mem[t], heap_extra_mem[t], peak_rss[t]);
        }
    }
    if (csv) fclose(csv);

    /* Growth Ratio Analysis */
    printf("\n========================================================================================================\n");
    printf("CUBIC SCALING JUSTIFICATION FOR FLOYD-WARSHALL:\n");
    double fw_growth = fw_times[1] / fw_times[0];
    printf(" - Measured time at V = 100   : %.6f seconds\n", fw_times[0]);
    printf(" - Measured time at V = 1,000 : %.6f seconds\n", fw_times[1]);
    printf(" - Empirical growth factor (100 -> 1,000): %.1fx (Theoretical O(V^3) growth = 10^3 = 1000.0x)\n", fw_growth);
    printf(" - Predicted time at V = 10,000: %.6f s * 1000 = %.3f seconds (~%.1f minutes / %.2f hours)\n",
           fw_times[1], fw_times[2], fw_times[2] / 60.0, fw_times[2] / 3600.0);
    printf(" - Memory requirement at V = 10,000 for flat matrix: 10,000^2 * 4 bytes = 400 MB\n");
    printf("========================================================================================================\n");

    return 0;
}
