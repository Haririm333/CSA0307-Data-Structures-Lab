#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../common/timer.h"
#include "../common/graph_gen.h"

/* BFS on Adjacency Matrix (1 byte per entry) */
static void bfs_matrix(int n, const unsigned char *mat, int src, int *dist) {
    for (int i = 0; i < n; i++) dist[i] = -1;
    int *queue = (int *)malloc(sizeof(int) * (size_t)n);
    if (!queue) return;

    int head = 0, tail = 0;
    dist[src] = 0;
    queue[tail++] = src;

    while (head < tail) {
        int u = queue[head++];
        const unsigned char *row = &mat[(size_t)u * (size_t)n];
        for (int v = 0; v < n; v++) {
            if (row[v] && dist[v] == -1) {
                dist[v] = dist[u] + 1;
                queue[tail++] = v;
            }
        }
    }
    free(queue);
}

/* BFS on CSR Adjacency List */
static void bfs_csr(const CSRGraph *g, int src, int *dist) {
    int n = g->n;
    for (int i = 0; i < n; i++) dist[i] = -1;
    int *queue = (int *)malloc(sizeof(int) * (size_t)n);
    if (!queue) return;

    int head = 0, tail = 0;
    dist[src] = 0;
    queue[tail++] = src;

    while (head < tail) {
        int u = queue[head++];
        for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
            int v = g->adj[idx];
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                queue[tail++] = v;
            }
        }
    }
    free(queue);
}

typedef struct {
    int n;
    const unsigned char *mat;
    const CSRGraph *g;
    int *dist;
    bool is_mat;
} BFSBenchArg;

static double bench_bfs(void *arg) {
    BFSBenchArg *b = (BFSBenchArg *)arg;
    double t0 = get_time_sec();
    if (b->is_mat) {
        bfs_matrix(b->n, b->mat, 0, b->dist);
    } else {
        bfs_csr(b->g, 0, b->dist);
    }
    double t1 = get_time_sec();
    return t1 - t0;
}

int main(void) {
    printf("========================================================================================================\n");
    printf("              GRAPH REPRESENTATION COMPARISON: ADJACENCY MATRIX vs ADJACENCY LIST (CSR)                 \n");
    printf("========================================================================================================\n\n");

    int test_v[] = {1000, 10000, 50000};
    int num_tests = sizeof(test_v) / sizeof(test_v[0]);

    FILE *csv = fopen("results.csv", "w");
    if (csv) {
        fprintf(csv, "V,E,Matrix_Time_s,List_Time_s,Matrix_Mem_Bytes,List_Mem_Bytes,Peak_RSS_KB\n");
    }

    printf("%-7s | %-9s | %-16s | %-16s | %-15s | %-15s | %-11s\n",
           "V", "E(dir)", "Matrix BFS (s)", "CSR List BFS (s)", "Matrix Mem", "List Mem", "Peak RSS");
    printf("--------------------------------------------------------------------------------------------------------\n");

    for (int t = 0; t < num_tests; t++) {
        int n = test_v[t];
        unsigned int seed = 42 + (unsigned int)t;
        CSRGraph *g = generate_random_connected_csr(n, 3, 1, 1, seed);
        if (!g) continue;

        size_t list_bytes = csr_analytic_bytes(g->n, g->m, false);
        size_t mat_bytes = (size_t)n * (size_t)n * sizeof(unsigned char);

        int *dist_list = (int *)malloc(sizeof(int) * (size_t)n);
        BFSBenchArg b_list = {n, NULL, g, dist_list, false};
        double t_list = run_median_3(bench_bfs, &b_list);

        /* Attempt allocation of adjacency matrix */
        unsigned char *mat = (unsigned char *)calloc((size_t)n * (size_t)n, sizeof(unsigned char));
        double t_mat = -1.0;
        bool mat_ok = false;

        if (mat) {
            mat_ok = true;
            /* Populate matrix from CSR */
            for (int u = 0; u < n; u++) {
                for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
                    int v = g->adj[idx];
                    mat[(size_t)u * (size_t)n + (size_t)v] = 1;
                }
            }
            int *dist_mat = (int *)malloc(sizeof(int) * (size_t)n);
            BFSBenchArg b_mat = {n, mat, NULL, dist_mat, true};
            t_mat = run_median_3(bench_bfs, &b_mat);
            free(dist_mat);
            free(mat);
        } else {
            printf("[Notice: V=%d calloc failed, analytic size = %.2f GB]\n", n, (double)mat_bytes / (1024.0 * 1024.0 * 1024.0));
        }

        long peak_rss = get_peak_rss_kb();

        char mat_time_str[32];
        if (mat_ok) {
            snprintf(mat_time_str, sizeof(mat_time_str), "%.6f s", t_mat);
        } else {
            snprintf(mat_time_str, sizeof(mat_time_str), "N/A (alloc fail)");
        }

        char mat_mem_str[32];
        if (mat_bytes >= 1024 * 1024 * 1024) {
            snprintf(mat_mem_str, sizeof(mat_mem_str), "%.2f GB", (double)mat_bytes / (1024.0 * 1024.0 * 1024.0));
        } else {
            snprintf(mat_mem_str, sizeof(mat_mem_str), "%.2f MB", (double)mat_bytes / (1024.0 * 1024.0));
        }

        char list_mem_str[32];
        if (list_bytes >= 1024 * 1024) {
            snprintf(list_mem_str, sizeof(list_mem_str), "%.2f MB", (double)list_bytes / (1024.0 * 1024.0));
        } else {
            snprintf(list_mem_str, sizeof(list_mem_str), "%.2f KB", (double)list_bytes / 1024.0);
        }

        printf("%-7d | %-9d | %-16s | %-14.6f s | %-15s | %-15s | %ld KB\n",
               n, g->m, mat_time_str, t_list, mat_mem_str, list_mem_str, peak_rss);

        if (csv) {
            if (mat_ok) {
                fprintf(csv, "%d,%d,%.6f,%.6f,%zu,%zu,%ld\n", n, g->m, t_mat, t_list, mat_bytes, list_bytes, peak_rss);
            } else {
                fprintf(csv, "%d,%d,FAILED,%.6f,%zu,%zu,%ld\n", n, g->m, t_list, mat_bytes, list_bytes, peak_rss);
            }
        }

        free(dist_list);
        csr_free(g);
    }

    if (csv) fclose(csv);
    printf("========================================================================================================\n");
    return 0;
}
