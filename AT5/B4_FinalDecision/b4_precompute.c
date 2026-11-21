#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#include "../common/timer.h"
#include "../common/minheap.h"
#include "../common/graph_gen.h"

#define INF 999999999
#define K 200
#define NUM_LOOKUPS 1000000

/* Generate 250,000-vertex grid-shortcut network */
static CSRGraph *generate_250k_road_network(unsigned int seed) {
    int n = 250000;
    int target_edges = (n * 3) / 2;
    srand(seed);

    int grid_dim = (int)ceil(sqrt(n));
    Edge *edges = (Edge *)malloc(sizeof(Edge) * (size_t)target_edges);
    if (!edges) return NULL;
    int e_count = 0;

    for (int y = 0; y < grid_dim; y++) {
        for (int x = 0; x < grid_dim; x++) {
            int u = y * grid_dim + x;
            if (u >= n) continue;
            if (x + 1 < grid_dim && e_count < target_edges) {
                int v = y * grid_dim + (x + 1);
                if (v < n) edges[e_count++] = (Edge){u, v, 1 + rand() % 100};
            }
            if (y + 1 < grid_dim && e_count < target_edges) {
                int v = (y + 1) * grid_dim + x;
                if (v < n) edges[e_count++] = (Edge){u, v, 1 + rand() % 100};
            }
        }
    }
    while (e_count < target_edges) {
        int u = rand() % n;
        int v = rand() % n;
        if (u != v) edges[e_count++] = (Edge){u, v, 5 + rand() % 95};
    }

    CSRGraph *g = build_csr_from_edges(n, e_count, edges, true);
    free(edges);
    return g;
}

/* Full Single-Source Dijkstra */
static void dijkstra_full(const CSRGraph *g, int src, int *dist) {
    int n = g->n;
    for (int i = 0; i < n; i++) dist[i] = INF;
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
            int w = g->weight[idx];
            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                minheap_push(heap, v, dist[v]);
            }
        }
    }
    minheap_free(heap);
}

/* Point-to-Point Dijkstra with Early Exit */
static int dijkstra_point_to_point(const CSRGraph *g, int src, int dst) {
    int n = g->n;
    int *dist = (int *)malloc(sizeof(int) * (size_t)n);
    bool *visited = (bool *)calloc((size_t)n, sizeof(bool));
    for (int i = 0; i < n; i++) dist[i] = INF;
    dist[src] = 0;

    MinHeap *heap = minheap_create(n);
    minheap_push(heap, src, 0);

    int found_dist = -1;

    while (!minheap_is_empty(heap)) {
        int u = 0;
        long long d = 0;
        if (!minheap_pop(heap, &u, &d)) break;

        if (visited[u]) continue;
        visited[u] = true;

        if (u == dst) {
            found_dist = dist[u];
            break; /* Early termination */
        }

        for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
            int v = g->adj[idx];
            int w = g->weight[idx];
            if (!visited[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                minheap_push(heap, v, dist[v]);
            }
        }
    }

    free(dist);
    free(visited);
    minheap_free(heap);
    return found_dist;
}

int main(void) {
    printf("========================================================================================================\n");
    printf("       B4.2 ALT: PRECOMPUTATION STRATEGY FOR K = 200 MAJOR JUNCTIONS (250K NETWORK)                     \n");
    printf("========================================================================================================\n\n");

    printf("Generating synthetic 250,000-junction road network (~750,000 edges)...\n");
    CSRGraph *g = generate_250k_road_network(42);
    if (!g) return 1;

    /* Select K = 200 major junctions */
    srand(2026);
    int hubs[K];
    for (int i = 0; i < K; i++) {
        hubs[i] = rand() % g->n;
    }

    printf("Selected K = %d major urban distribution hubs across the city map.\n\n", K);

    /* Phase 1: Precomputation of K x K Distance Table */
    printf("--- PHASE 1: Table Precomputation (K = %d Dijkstra Runs) ---\n", K);
    int *table = (int *)malloc(sizeof(int) * (size_t)(K * K));
    int *full_dist = (int *)malloc(sizeof(int) * (size_t)g->n);

    double t_pre_start = get_time_sec();
    for (int i = 0; i < K; i++) {
        dijkstra_full(g, hubs[i], full_dist);
        for (int j = 0; j < K; j++) {
            table[i * K + j] = full_dist[hubs[j]];
        }
    }
    double t_pre_end = get_time_sec();
    double total_recompute_sec = t_pre_end - t_pre_start;

    printf("Total Precomputation Time: %.3f seconds (%.2f ms per hub)\n",
           total_recompute_sec, (total_recompute_sec / (double)K) * 1000.0);
    printf("Budget Utilization: consumes %.2f%% of the 2-minute (120 s) scheduling window!\n\n",
           (total_recompute_sec / 120.0) * 100.0);

    /* Phase 2: Storage Footprint Analysis */
    printf("--- PHASE 2: Storage Overhead Analysis ---\n");
    size_t dist_only_bytes = (size_t)(K * K) * sizeof(int);
    size_t next_hop_bytes = (size_t)(K * K) * sizeof(int);
    size_t full_trees_bytes = (size_t)K * (size_t)g->n * sizeof(int);

    printf("1. Distance-Only Matrix (K x K): %zu bytes (%.2f KB / %.3f MB)\n",
           dist_only_bytes, (double)dist_only_bytes / 1024.0, (double)dist_only_bytes / (1024.0 * 1024.0));
    printf("2. Distances + Hub-to-Hub Next-Hop Table: %zu bytes (%.2f KB / %.3f MB)\n",
           dist_only_bytes + next_hop_bytes,
           (double)(dist_only_bytes + next_hop_bytes) / 1024.0,
           (double)(dist_only_bytes + next_hop_bytes) / (1024.0 * 1024.0));
    printf("3. Complete Precomputed Path Trees (K x |V| full parent pointers): %zu bytes (%.2f MB)\n\n",
           full_trees_bytes, (double)full_trees_bytes / (1024.0 * 1024.0));

    /* Phase 3: Query Throughput Comparison */
    printf("--- PHASE 3: Request Latency Comparison (Table Lookup vs Fresh Point-to-Point Dijkstra) ---\n");

    /* Benchmark 1,000,000 Table Lookups */
    volatile int dummy = 0;
    double t_look0 = get_time_sec();
    for (int i = 0; i < NUM_LOOKUPS; i++) {
        int h1 = i % K;
        int h2 = (i * 7 + 13) % K;
        dummy += table[h1 * K + h2];
    }
    double t_look1 = get_time_sec();
    (void)dummy;
    double total_look_sec = t_look1 - t_look0;
    double ns_per_lookup = (total_look_sec / (double)NUM_LOOKUPS) * 1e9;

    printf("1. Precomputed Table Lookup (Tested across %'d queries):\n", NUM_LOOKUPS);
    printf("   - Total Time     : %.4f seconds\n", total_look_sec);
    printf("   - Latency/Query  : %.2f nanoseconds per lookup\n", ns_per_lookup);
    printf("   - Throughput     : %.1f million queries / second\n\n", (double)NUM_LOOKUPS / total_look_sec / 1e6);

    /* Benchmark Fresh Point-to-Point Dijkstra (5 queries) */
    printf("2. Fresh Point-to-Point Dijkstra (with early goal exit):\n");
    double p2p_times[5];
    for (int i = 0; i < 5; i++) {
        int src = hubs[i];
        int dst = hubs[(i + 50) % K];
        double t0 = get_time_sec();
        int d = dijkstra_point_to_point(g, src, dst);
        double t1 = get_time_sec();
        p2p_times[i] = t1 - t0;
        printf("   - Query %d (Hub %d -> Hub %d): dist = %d, latency = %.2f ms\n",
               i + 1, src, dst, d, (t1 - t0) * 1000.0);
    }
    qsort(p2p_times, 5, sizeof(double), compare_doubles);
    double median_p2p_ms = p2p_times[2] * 1000.0;
    printf("   - Median Fresh Query Latency: %.2f ms\n\n", median_p2p_ms);

    double speedup = (median_p2p_ms * 1e6) / ns_per_lookup;
    printf("========================================================================================================\n");
    printf("SPEEDUP FACTOR: Table lookup is %.0fx faster than fresh point-to-point Dijkstra!\n", speedup);
    printf("ENGINEERING CONCLUSION:\n");
    printf("Precomputing the %d x %d distance matrix requires merely %.2f KB of memory and consumes only\n",
           K, K, (double)dist_only_bytes / 1024.0);
    printf("%.2f%% of the 2-minute batch update window (%.2fs / 120s).\n",
           (total_recompute_sec / 120.0) * 100.0, total_recompute_sec);
    printf("This delivers nanosecond dispatch responses for real-time delivery routing APIs.\n");
    printf("========================================================================================================\n");

    free(full_dist);
    free(table);
    csr_free(g);
    return 0;
}
