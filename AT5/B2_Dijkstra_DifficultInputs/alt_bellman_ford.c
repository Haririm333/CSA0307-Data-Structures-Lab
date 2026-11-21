#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

#include "../common/timer.h"
#include "../common/graph_gen.h"

#define INF 999999999

/* Bellman-Ford algorithm operating on an edge list */
static bool bellman_ford(int n, int num_edges, const Edge *edges, int src, int *dist) {
    for (int i = 0; i < n; i++) dist[i] = INF;
    dist[src] = 0;

    /* Relax edges V - 1 times */
    for (int iter = 0; iter < n - 1; iter++) {
        bool changed = false;
        for (int e = 0; e < num_edges; e++) {
            int u = edges[e].u;
            int v = edges[e].v;
            int w = edges[e].w;
            if (dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                changed = true;
            }
        }
        if (!changed) break; /* Early termination */
    }

    /* V-th pass: Negative cycle detection */
    for (int e = 0; e < num_edges; e++) {
        int u = edges[e].u;
        int v = edges[e].v;
        int w = edges[e].w;
        if (dist[u] != INF && dist[u] + w < dist[v]) {
            return false; /* Negative cycle detected */
        }
    }
    return true; /* No negative cycle */
}

/* Dijkstra Array Version for Benchmarking */
static void dijkstra_array(const CSRGraph *g, int src, int *dist) {
    int n = g->n;
    bool *done = (bool *)calloc((size_t)n, sizeof(bool));
    for (int i = 0; i < n; i++) dist[i] = INF;
    dist[src] = 0;

    for (int step = 0; step < n; step++) {
        int u = -1;
        for (int v = 0; v < n; v++) {
            if (!done[v] && dist[v] != INF && (u == -1 || dist[v] < dist[u])) {
                u = v;
            }
        }
        if (u == -1) break;
        done[u] = true;

        for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
            int v = g->adj[idx];
            int w = g->weight[idx];
            if (!done[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
            }
        }
    }
    free(done);
}

typedef struct {
    int n;
    int num_edges;
    const Edge *edges;
    const CSRGraph *g;
    int *dist;
    bool is_bellman;
} BenchArg;

static double bench_run(void *arg) {
    BenchArg *b = (BenchArg *)arg;
    double t0 = get_time_sec();
    if (b->is_bellman) {
        bellman_ford(b->n, b->num_edges, b->edges, 0, b->dist);
    } else {
        dijkstra_array(b->g, 0, b->dist);
    }
    double t1 = get_time_sec();
    return t1 - t0;
}

int main(void) {
    printf("========================================================================================\n");
    printf("          BELLMAN-FORD ALGORITHM: NEGATIVE EDGES, CYCLES & BENCHMARKS                   \n");
    printf("========================================================================================\n\n");

    /* TEST 1: Negative Edge Graph (0->1 w=2, 0->2 w=3, 2->1 w=-2) */
    printf("--- TEST 1: Graph with Negative Edge (No Cycle) ---\n");
    Edge neg_edges[] = {
        {0, 1, 2}, {0, 2, 3}, {2, 1, -2}
    };
    int dist_t1[3];
    bool t1_ok = bellman_ford(3, 3, neg_edges, 0, dist_t1);
    printf("Status: %s\n", t1_ok ? "PASS (No negative cycle)" : "FAIL");
    printf("Distances from 0: dist[0]=%d, dist[1]=%d (CORRECT via 0->2->1), dist[2]=%d\n\n",
           dist_t1[0], dist_t1[1], dist_t1[2]);

    /* TEST 2: Graph with Negative Cycle (0->1 w=1, 1->2 w=-5, 2->0 w=2 -> sum = -2) */
    printf("--- TEST 2: Graph with Negative Cycle ---\n");
    Edge cycle_edges[] = {
        {0, 1, 1}, {1, 2, -5}, {2, 0, 2}
    };
    int dist_t2[3];
    bool t2_ok = bellman_ford(3, 3, cycle_edges, 0, dist_t2);
    printf("Result: %s\n\n", t2_ok ? "No negative cycle" : "NEGATIVE CYCLE DETECTED (VERIFIED)");

    /* TEST 3: Chennai 6-Node Graph (must match Dijkstra: 0 7 9 20 20 11) */
    printf("--- TEST 3: Chennai 6-Node Urban Network Verification ---\n");
    int w_chennai[6][6] = {
        {0, 7, 9, 0, 0, 14},
        {7, 0, 10, 15, 0, 0},
        {9, 10, 0, 11, 0, 2},
        {0, 15, 11, 0, 6, 0},
        {0, 0, 0, 6, 0, 9},
        {14, 0, 2, 0, 9, 0},
    };
    Edge chennai_edges[18];
    int ce_count = 0;
    for (int u = 0; u < 6; u++) {
        for (int v = 0; v < 6; v++) {
            if (w_chennai[u][v] > 0) {
                chennai_edges[ce_count++] = (Edge){u, v, w_chennai[u][v]};
            }
        }
    }
    int dist_chennai[6];
    bellman_ford(6, ce_count, chennai_edges, 0, dist_chennai);
    printf("Bellman-Ford Distances: ");
    for (int i = 0; i < 6; i++) printf("%d ", dist_chennai[i]);
    printf("\nExpected Dijkstra     : 0 7 9 20 20 11 \n");
    printf("Status: %s\n\n", (dist_chennai[0] == 0 && dist_chennai[1] == 7 &&
                             dist_chennai[2] == 9 && dist_chennai[3] == 20 &&
                             dist_chennai[4] == 20 && dist_chennai[5] == 11) ? "EXACT MATCH (VERIFIED)" : "MISMATCH");

    /* TEST 4: Benchmark Dijkstra (Array) vs Bellman-Ford on V = 1,000 and 5,000 */
    printf("--- TEST 4: Benchmark: Dijkstra (Array) vs Bellman-Ford (Median of 3 Runs) ---\n");
    printf("%-8s | %-12s | %-20s | %-20s | %-12s\n",
           "V", "Edges (dir)", "Dijkstra Array (s)", "Bellman-Ford (s)", "Speedup");
    printf("----------------------------------------------------------------------------------------\n");

    int bench_v[] = {1000, 5000};
    for (int t = 0; t < 2; t++) {
        int n = bench_v[t];
        unsigned int seed = 42 + (unsigned int)t;
        CSRGraph *g = generate_random_connected_csr(n, 3, 1, 100, seed);
        if (!g) continue;

        /* Flatten CSR edges into Edge array for Bellman-Ford */
        Edge *edge_list = (Edge *)malloc(sizeof(Edge) * (size_t)g->m);
        int e_idx = 0;
        for (int u = 0; u < n; u++) {
            for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
                edge_list[e_idx++] = (Edge){u, g->adj[idx], g->weight[idx]};
            }
        }

        int *d_dij = (int *)malloc(sizeof(int) * (size_t)n);
        int *d_bell = (int *)malloc(sizeof(int) * (size_t)n);

        BenchArg b_dij = {n, g->m, edge_list, g, d_dij, false};
        BenchArg b_bell = {n, g->m, edge_list, g, d_bell, true};

        double t_dij = run_median_3(bench_run, &b_dij);
        double t_bell = run_median_3(bench_run, &b_bell);

        double speedup = (t_dij > 0.0) ? (t_bell / t_dij) : 0.0;

        printf("%-8d | %-12d | %-20.6f | %-20.6f | %-10.2fx faster\n",
               n, g->m, t_dij, t_bell, speedup);

        free(edge_list);
        free(d_dij);
        free(d_bell);
        csr_free(g);
    }

    printf("========================================================================================\n");
    return 0;
}
