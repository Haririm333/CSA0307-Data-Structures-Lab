#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#include "../common/timer.h"
#include "../common/minheap.h"
#include "../common/graph_gen.h"

/* Unit-weight Dijkstra using binary min-heap */
static void dijkstra_unit(const CSRGraph *g, int src, int *dist) {
    int n = g->n;
    for (int i = 0; i < n; i++) dist[i] = INT_MAX;
    dist[src] = 0;

    MinHeap *heap = minheap_create(n * 2);
    minheap_push(heap, src, 0);

    while (!minheap_is_empty(heap)) {
        int u = 0;
        long long d = 0;
        if (!minheap_pop(heap, &u, &d)) break;

        if (d > dist[u]) continue;

        for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
            int v = g->adj[idx];
            int w = 1; /* Unit weight */
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                minheap_push(heap, v, dist[v]);
            }
        }
    }

    minheap_free(heap);
}

/* BFS on CSR Graph */
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
    const CSRGraph *g;
    int *dist;
    bool is_dijkstra;
} BenchArg;

static double bench_run(void *arg) {
    BenchArg *b = (BenchArg *)arg;
    double t0 = get_time_sec();
    if (b->is_dijkstra) {
        dijkstra_unit(b->g, 0, b->dist);
    } else {
        bfs_csr(b->g, 0, b->dist);
    }
    double t1 = get_time_sec();
    return t1 - t0;
}

int main(void) {
    printf("========================================================================================\n");
    printf("           FEWEST-STOPS: BFS vs UNIT-WEIGHT DIJKSTRA (BINARY HEAP)                      \n");
    printf("========================================================================================\n\n");

    /* Part 1: Verification on Chennai 7-Station Metro */
    printf("--- PART 1: Verification on Chennai Metro Graph ---\n");
    Edge metro_edges[] = {
        {0, 1, 1}, {1, 2, 1}, {1, 5, 1}, {2, 3, 1},
        {3, 4, 1}, {4, 6, 1}, {5, 6, 1}
    };
    int num_metro = sizeof(metro_edges) / sizeof(metro_edges[0]);
    CSRGraph *g_metro = build_csr_from_edges(7, num_metro, metro_edges, true);

    int bfs_dist[7];
    int dij_dist[7];
    bfs_csr(g_metro, 0, bfs_dist);
    dijkstra_unit(g_metro, 0, dij_dist);

    bool match = true;
    for (int i = 0; i < 7; i++) {
        if (bfs_dist[i] != dij_dist[i]) {
            match = false;
            break;
        }
    }
    printf("BFS Distances from Airport (0): [ ");
    for (int i = 0; i < 7; i++) printf("%d ", bfs_dist[i]);
    printf("]\n");

    printf("Unit Dijkstra Distances       : [ ");
    for (int i = 0; i < 7; i++) printf("%d ", dij_dist[i]);
    printf("]\n");

    printf("Result: %s\n\n", match ? "EXACT MATCH (VERIFIED)" : "MISMATCH");
    csr_free(g_metro);

    /* Part 2: Benchmarking at 10,000 and 50,000 vertices */
    printf("--- PART 2: Benchmark Scaling on Large Graphs (Median of 3 Runs) ---\n");
    printf("%-8s | %-12s | %-16s | %-16s | %-12s | %-16s\n",
           "V", "E(dir)", "BFS Time (s)", "Dijkstra Time(s)", "BFS Overhead", "Heap Extra Space");
    printf("----------------------------------------------------------------------------------------\n");

    int test_v[] = {10000, 50000};
    for (int t = 0; t < 2; t++) {
        int n = test_v[t];
        unsigned int seed = 100 + (unsigned int)t;
        CSRGraph *g = generate_random_connected_csr(n, 3, 1, 1, seed);
        if (!g) continue;

        int *d_bfs = (int *)malloc(sizeof(int) * (size_t)n);
        int *d_dij = (int *)malloc(sizeof(int) * (size_t)n);

        BenchArg b_bfs = {g, d_bfs, false};
        BenchArg b_dij = {g, d_dij, true};

        double t_bfs = run_median_3(bench_run, &b_bfs);
        double t_dij = run_median_3(bench_run, &b_dij);

        size_t heap_extra = sizeof(MinHeap) + (size_t)(2 * n) * sizeof(HeapNode);
        double overhead = (t_bfs > 0.0) ? (t_dij / t_bfs) : 0.0;

        printf("%-8d | %-12d | %-16.6f | %-16.6f | %-10.2fx slower | %zu KB\n",
               n, g->m, t_bfs, t_dij, overhead, heap_extra / 1024);

        free(d_bfs);
        free(d_dij);
        csr_free(g);
    }

    printf("========================================================================================\n");
    printf("TAKEAWAY: For unweighted/unit-weight graphs, standard FIFO queue BFS runs in strictly\n");
    printf("O(V + E) linear time. A binary heap introduces an unnecessary O(log V) priority queue\n");
    printf("overhead per edge relaxation, making Dijkstra substantially slower and more memory-heavy.\n");
    printf("========================================================================================\n");

    return 0;
}
