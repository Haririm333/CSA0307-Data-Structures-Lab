#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

#include "../common/timer.h"
#include "../common/minheap.h"
#include "../common/graph_gen.h"

#define INF 999999999

/* Run Floyd-Warshall on an n x n adjacency matrix */
static void run_floyd_warshall(int n, const int *adj_matrix, int *dist_out) {
    memcpy(dist_out, adj_matrix, sizeof(int) * (size_t)(n * n));
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            int dik = dist_out[i * n + k];
            if (dik == INF) continue;
            for (int j = 0; j < n; j++) {
                int dkj = dist_out[k * n + j];
                if (dkj == INF) continue;
                if (dik + dkj < dist_out[i * n + j]) {
                    dist_out[i * n + j] = dik + dkj;
                }
            }
        }
    }
}

/* Single-source Dijkstra using CSR graph and binary min-heap */
static void dijkstra_csr_single(const CSRGraph *g, int src, int *dist_out) {
    int n = g->n;
    for (int i = 0; i < n; i++) dist_out[i] = INF;
    dist_out[src] = 0;

    MinHeap *heap = minheap_create(n * 2);
    minheap_push(heap, src, 0);

    while (!minheap_is_empty(heap)) {
        int u = 0;
        long long d = 0;
        if (!minheap_pop(heap, &u, &d)) break;

        if (d > dist_out[u]) continue;

        for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
            int v = g->adj[idx];
            int w = g->weight[idx];
            if (dist_out[u] + w < dist_out[v]) {
                dist_out[v] = dist_out[u] + w;
                minheap_push(heap, v, dist_out[v]);
            }
        }
    }

    minheap_free(heap);
}

/* All-pairs Dijkstra: run single-source Dijkstra from each vertex */
static void run_all_pairs_dijkstra(const CSRGraph *g, int *dist_out) {
    int n = g->n;
    for (int i = 0; i < n; i++) {
        dijkstra_csr_single(g, i, &dist_out[i * n]);
    }
}

/* Benchmark harness wrapper */
typedef struct {
    int n;
    const int *mat;
    const CSRGraph *g;
    int *out;
    bool is_floyd;
} BenchArg;

static double bench_run(void *arg) {
    BenchArg *b = (BenchArg *)arg;
    double t0 = get_time_sec();
    if (b->is_floyd) {
        run_floyd_warshall(b->n, b->mat, b->out);
    } else {
        run_all_pairs_dijkstra(b->g, b->out);
    }
    double t1 = get_time_sec();
    return t1 - t0;
}

int main(void) {
    printf("========================================================================================\n");
    printf("           ALL-PAIRS SHORTEST PATH: FLOYD-WARSHALL vs ALL-PAIRS DIJKSTRA                \n");
    printf("========================================================================================\n\n");

    /* Part 1: Correctness verification on 4-vertex assignment graph */
    printf("--- PART 1: Verification on Task A4 4-Vertex Graph ---\n");
    int n4 = 4;
    int a4_mat[16] = {
        0, 5, INF, 10,
        INF, 0, 3, INF,
        INF, INF, 0, 1,
        INF, INF, INF, 0
    };

    Edge a4_edges[4] = {
        {0, 1, 5},
        {0, 3, 10},
        {1, 2, 3},
        {2, 3, 1}
    };
    CSRGraph *g4 = build_csr_from_edges(n4, 4, a4_edges, false);

    int fw_res[16];
    int dij_res[16];
    run_floyd_warshall(n4, a4_mat, fw_res);
    run_all_pairs_dijkstra(g4, dij_res);

    bool match = true;
    for (int i = 0; i < 16; i++) {
        if (fw_res[i] != dij_res[i]) {
            match = false;
            break;
        }
    }
    printf("Floyd-Warshall 4x4 Table vs All-Pairs Dijkstra: %s\n", match ? "EXACT MATCH (VERIFIED)" : "MISMATCH");
    for (int i = 0; i < n4; i++) {
        printf("Row %d: FW=[", i);
        for (int j = 0; j < n4; j++) printf(" %4d", fw_res[i * n4 + j] >= 999 ? 999 : fw_res[i * n4 + j]);
        printf(" ] | Dijkstra=[");
        for (int j = 0; j < n4; j++) printf(" %4d", dij_res[i * n4 + j] >= 999 ? 999 : dij_res[i * n4 + j]);
        printf(" ]\n");
    }
    csr_free(g4);

    /* Part 2: Benchmark scaling on random graphs: 30, 200, 400, 800 vertices */
    printf("\n--- PART 2: Benchmark Scaling (Average degree ~3 edges/vertex, median of 3 runs) ---\n");
    printf("%-8s | %-12s | %-20s | %-20s | %-10s\n", "Vertices", "Edges (CSR)", "Floyd-Warshall (s)", "All-Pairs Dijkstra(s)", "Speedup");
    printf("----------------------------------------------------------------------------------------\n");

    int test_sizes[] = {30, 200, 400, 800};
    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);

    for (int t = 0; t < num_tests; t++) {
        int V_cur = test_sizes[t];
        unsigned int seed = 42 + (unsigned int)t;
        CSRGraph *g = generate_random_connected_csr(V_cur, 3, 1, 100, seed);
        if (!g) continue;

        /* Create flat adjacency matrix for Floyd-Warshall */
        int *mat = (int *)malloc(sizeof(int) * (size_t)(V_cur * V_cur));
        for (int i = 0; i < V_cur * V_cur; i++) mat[i] = INF;
        for (int i = 0; i < V_cur; i++) mat[i * V_cur + i] = 0;
        for (int u = 0; u < V_cur; u++) {
            for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
                int v = g->adj[idx];
                int w = g->weight[idx];
                if (w < mat[u * V_cur + v]) mat[u * V_cur + v] = w;
            }
        }

        int *fw_out = (int *)malloc(sizeof(int) * (size_t)(V_cur * V_cur));
        int *dij_out = (int *)malloc(sizeof(int) * (size_t)(V_cur * V_cur));

        BenchArg b_fw = {V_cur, mat, g, fw_out, true};
        BenchArg b_dij = {V_cur, mat, g, dij_out, false};

        double t_fw = run_median_3(bench_run, &b_fw);
        double t_dij = run_median_3(bench_run, &b_dij);

        double speedup = (t_dij > 0.0) ? (t_fw / t_dij) : 0.0;

        printf("%-8d | %-12d | %-20.6f | %-20.6f | %-10.2fx\n",
               V_cur, g->m, t_fw, t_dij, speedup);

        free(mat);
        free(fw_out);
        free(dij_out);
        csr_free(g);
    }

    printf("========================================================================================\n");
    return 0;
}
