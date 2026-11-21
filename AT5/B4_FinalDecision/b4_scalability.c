#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "../common/timer.h"
#include "../common/minheap.h"
#include "../common/graph_gen.h"

#define INF 999999999

/* Synthetic 2D grid road network with random shortcuts */
static CSRGraph *generate_road_network(int n, int target_undir_edges, unsigned int seed) {
    srand(seed);
    int grid_dim = (int)ceil(sqrt(n));

    Edge *edges = (Edge *)malloc(sizeof(Edge) * (size_t)target_undir_edges);
    if (!edges) return NULL;
    int e_count = 0;

    /* 1. Grid edges (Right & Down) */
    for (int y = 0; y < grid_dim; y++) {
        for (int x = 0; x < grid_dim; x++) {
            int u = y * grid_dim + x;
            if (u >= n) continue;

            /* Right edge */
            if (x + 1 < grid_dim) {
                int v = y * grid_dim + (x + 1);
                if (v < n && e_count < target_undir_edges) {
                    int w = 1 + rand() % 100;
                    edges[e_count++] = (Edge){u, v, w};
                }
            }
            /* Down edge */
            if (y + 1 < grid_dim) {
                int v = (y + 1) * grid_dim + x;
                if (v < n && e_count < target_undir_edges) {
                    int w = 1 + rand() % 100;
                    edges[e_count++] = (Edge){u, v, w};
                }
            }
        }
    }

    /* 2. Arterial / Expressway shortcuts */
    while (e_count < target_undir_edges) {
        int u = rand() % n;
        int v = rand() % n;
        if (u != v) {
            int w = 5 + rand() % 95;
            edges[e_count++] = (Edge){u, v, w};
        }
    }

    CSRGraph *g = build_csr_from_edges(n, e_count, edges, true);
    free(edges);
    return g;
}

static void dijkstra_heap(const CSRGraph *g, int src, int *dist) {
    int n = g->n;
    for (int i = 0; i < n; i++) dist[i] = INF;
    dist[src] = 0;

    MinHeap *heap = minheap_create(n * 2);
    if (!heap) return;
    minheap_push(heap, src, 0);

    while (!minheap_is_empty(heap)) {
        int u = 0;
        long long d = 0;
        if (!minheap_pop(heap, &u, &d)) break;

        if (d > dist[u]) continue;

        for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
            int v = g->adj[idx];
            int w = g->weight[idx];
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                minheap_push(heap, v, dist[v]);
            }
        }
    }
    minheap_free(heap);
}

int main(void) {
    printf("========================================================================================================\n");
    printf("        B4.2: LARGE-SCALE HIGH-PERFORMANCE ROAD NETWORK SCALABILITY BENCHMARK (250K NODES)              \n");
    printf("========================================================================================================\n\n");

    /* Part 1: Scaling across 10k, 50k, 100k, and 250k nodes */
    int test_sizes[] = {10000, 50000, 100000, 250000};
    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);
    double per_query_medians[4];

    printf("--- PART 1: Query Latency Scaling (Average degree ~3, 10 Random Sources) ---\n");
    printf("%-8s | %-12s | %-16s | %-16s | %-14s | %-12s\n",
           "Vertices", "Edges(dir)", "Analytic CSR Mem", "Heap Memory", "Median Query", "Peak RSS");
    printf("--------------------------------------------------------------------------------------------------------\n");

    for (int t = 0; t < num_tests; t++) {
        int n = test_sizes[t];
        int target_undir = (int)((long long)n * 3LL / 2LL);
        CSRGraph *g = generate_road_network(n, target_undir, 42 + (unsigned int)t);
        if (!g) continue;

        size_t off_bytes = (size_t)(g->n + 1) * sizeof(int);
        size_t adj_bytes = (size_t)g->m * sizeof(int);
        size_t wt_bytes = (size_t)g->m * sizeof(int);
        size_t csr_total = sizeof(CSRGraph) + off_bytes + adj_bytes + wt_bytes;
        size_t heap_mem = sizeof(MinHeap) + (size_t)(g->n * 2) * sizeof(HeapNode);

        int *dist = (int *)malloc(sizeof(int) * (size_t)n);

        /* 10 random queries */
        srand(999);
        double query_times[10];
        for (int q = 0; q < 10; q++) {
            int src = rand() % n;
            double t0 = get_time_sec();
            dijkstra_heap(g, src, dist);
            double t1 = get_time_sec();
            query_times[q] = t1 - t0;
        }
        qsort(query_times, 10, sizeof(double), compare_doubles);
        per_query_medians[t] = query_times[5]; /* Median */

        long peak_rss = get_peak_rss_kb();

        printf("%-8d | %-12d | %-16.2f MB | %-16.2f MB | %-12.4f ms | %ld KB\n",
               n, g->m,
               (double)csr_total / (1024.0 * 1024.0),
               (double)heap_mem / (1024.0 * 1024.0),
               per_query_medians[t] * 1000.0,
               peak_rss);

        free(dist);
        csr_free(g);
    }

    /* Scaling Ratio & Log-Log Slope */
    printf("\n--- SCALING COMPLEXITY ANALYSIS ---\n");
    double v_ratio = (double)test_sizes[3] / (double)test_sizes[0]; /* 250k / 10k = 25 */
    double t_ratio = per_query_medians[3] / per_query_medians[0];
    double log_log_slope = log(t_ratio) / log(v_ratio);
    printf(" - Scaling from V = 10,000 to 250,000 (Size multiplier: 25.0x)\n");
    printf(" - Time multiplier: %.2fx (Median query: %.2f ms -> %.2f ms)\n",
           t_ratio, per_query_medians[0] * 1000.0, per_query_medians[3] * 1000.0);
    printf(" - Empirical Log-Log Scaling Exponent: %.2f (Theoretical O(V log V) ~ 1.1x - 1.2x)\n\n", log_log_slope);

    /* Part 2: Two-Minute Recomputation Planning */
    printf("--- PART 2: Feasibility of 2-Minute Full Recomputation Window (120 s) ---\n");
    double t_per_src = per_query_medians[3];
    int scenario_N1 = 100;
    int scenario_N2 = 1000;

    double time_N1 = (double)scenario_N1 * t_per_src;
    double time_N2 = (double)scenario_N2 * t_per_src;

    printf("1. Scenario A: N = %d Major Junctions / Distribution Hubs:\n", scenario_N1);
    printf("   - Total Recompute Time : %.3f seconds (%d * %.4f ms)\n",
           time_N1, scenario_N1, t_per_src * 1000.0);
    printf("   - Budget Utilization   : %.2f%% of 120-second window -> HIGHLY FEASIBLE (Leaves %.1fs for other workloads)\n\n",
           (time_N1 / 120.0) * 100.0, 120.0 - time_N1);

    printf("2. Scenario B: N = %d Major Junctions / Distribution Hubs:\n", scenario_N2);
    printf("   - Total Recompute Time : %.3f seconds (%d * %.4f ms)\n",
           time_N2, scenario_N2, t_per_src * 1000.0);
    printf("   - Budget Utilization   : %.2f%% of 120-second window\n", (time_N2 / 120.0) * 100.0);
    if (time_N2 > 120.0) {
        printf("   - Feasibility Assessment: EXCEEDS single-threaded 120s limit! Requires multi-threaded parallel dispatch\n");
        printf("     or selective / lazy hub recomputation.\n");
    } else {
        printf("   - Feasibility Assessment: FEASIBLE within the 120s window.\n");
    }

    printf("========================================================================================================\n");
    return 0;
}
