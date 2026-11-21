#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>

#include "../common/timer.h"
#include "../common/minheap.h"
#include "../common/graph_gen.h"

/* Disjoint Set Union (Union-Find) with path compression and union by rank */
typedef struct {
    int *parent;
    int *rank;
    int n;
} DSU;

static DSU *dsu_create(int n) {
    DSU *d = (DSU *)malloc(sizeof(DSU));
    if (!d) return NULL;
    d->n = n;
    d->parent = (int *)malloc(sizeof(int) * (size_t)n);
    d->rank = (int *)calloc((size_t)n, sizeof(int));
    if (!d->parent || !d->rank) {
        free(d->parent);
        free(d->rank);
        free(d);
        return NULL;
    }
    for (int i = 0; i < n; i++) d->parent[i] = i;
    return d;
}

static void dsu_free(DSU *d) {
    if (d) {
        free(d->parent);
        free(d->rank);
        free(d);
    }
}

static int dsu_find(DSU *d, int i) {
    if (d->parent[i] != i) {
        d->parent[i] = dsu_find(d, d->parent[i]); /* Path compression */
    }
    return d->parent[i];
}

static bool dsu_union(DSU *d, int x, int y) {
    int root_x = dsu_find(d, x);
    int root_y = dsu_find(d, y);
    if (root_x == root_y) return false;

    /* Union by rank */
    if (d->rank[root_x] < d->rank[root_y]) {
        d->parent[root_x] = root_y;
    } else if (d->rank[root_x] > d->rank[root_y]) {
        d->parent[root_y] = root_x;
    } else {
        d->parent[root_y] = root_x;
        d->rank[root_x]++;
    }
    return true;
}

static int compare_edges_by_weight(const void *a, const void *b) {
    const Edge *ea = (const Edge *)a;
    const Edge *eb = (const Edge *)b;
    return ea->w - eb->w;
}

/* Kruskal's MST algorithm */
static long long run_kruskal(int n, int num_edges, Edge *edges) {
    qsort(edges, (size_t)num_edges, sizeof(Edge), compare_edges_by_weight);
    DSU *d = dsu_create(n);
    if (!d) return -1;

    long long total_weight = 0;
    int edges_added = 0;

    for (int i = 0; i < num_edges; i++) {
        if (dsu_union(d, edges[i].u, edges[i].v)) {
            total_weight += edges[i].w;
            edges_added++;
            if (edges_added == n - 1) break;
        }
    }

    dsu_free(d);
    return total_weight;
}

/* Prim's MST: Array Version O(V^2) */
static long long run_prim_array(int n, const CSRGraph *g) {
    int *key = (int *)malloc(sizeof(int) * (size_t)n);
    bool *in_tree = (bool *)calloc((size_t)n, sizeof(bool));
    if (!key || !in_tree) {
        free(key);
        free(in_tree);
        return -1;
    }

    for (int i = 0; i < n; i++) key[i] = INT_MAX;
    key[0] = 0;
    long long total = 0;

    for (int step = 0; step < n; step++) {
        int u = -1;
        for (int v = 0; v < n; v++) {
            if (!in_tree[v] && (u == -1 || key[v] < key[u])) {
                u = v;
            }
        }
        if (u == -1 || key[u] == INT_MAX) break;
        in_tree[u] = true;
        total += key[u];

        for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
            int v = g->adj[idx];
            int w = g->weight[idx];
            if (!in_tree[v] && w < key[v]) {
                key[v] = w;
            }
        }
    }

    free(key);
    free(in_tree);
    return total;
}

/* Prim's MST: Binary Min-Heap Version O(E log V) */
static long long run_prim_heap(int n, const CSRGraph *g) {
    int *key = (int *)malloc(sizeof(int) * (size_t)n);
    bool *in_tree = (bool *)calloc((size_t)n, sizeof(bool));
    MinHeap *heap = minheap_create(g->m + 16);

    for (int i = 0; i < n; i++) key[i] = INT_MAX;
    key[0] = 0;
    minheap_push(heap, 0, 0);

    long long total = 0;

    while (!minheap_is_empty(heap)) {
        int u = 0;
        long long dist = 0;
        if (!minheap_pop(heap, &u, &dist)) break;

        if (in_tree[u]) continue;
        in_tree[u] = true;
        total += dist;

        for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
            int v = g->adj[idx];
            int w = g->weight[idx];
            if (!in_tree[v] && w < key[v]) {
                key[v] = w;
                minheap_push(heap, v, w);
            }
        }
    }

    free(key);
    free(in_tree);
    minheap_free(heap);
    return total;
}

/* Benchmark harness wrappers */
typedef struct {
    int n;
    int m;
    Edge *edges;
    CSRGraph *g;
    int mode; /* 0 = Prim array, 1 = Prim heap, 2 = Kruskal */
} MSTBenchArg;

static double bench_mst(void *arg) {
    MSTBenchArg *b = (MSTBenchArg *)arg;
    double t0 = get_time_sec();
    if (b->mode == 0) {
        run_prim_array(b->n, b->g);
    } else if (b->mode == 1) {
        run_prim_heap(b->n, b->g);
    } else {
        /* Make a copy of edges because Kruskal sorts in-place */
        Edge *cpy = (Edge *)malloc(sizeof(Edge) * (size_t)b->m);
        memcpy(cpy, b->edges, sizeof(Edge) * (size_t)b->m);
        run_kruskal(b->n, b->m, cpy);
        free(cpy);
    }
    double t1 = get_time_sec();
    return t1 - t0;
}

int main(void) {
    printf("========================================================================================\n");
    printf("           MINIMUM SPANNING TREE: PRIM (ARRAY/HEAP) vs KRUSKAL BENCHMARK                \n");
    printf("========================================================================================\n\n");

    /* Part 1: Verification on Task A5 5-Village Graph */
    printf("--- PART 1: Verification on Task A5 5-Village Fibre Network ---\n");
    int n5 = 5;
    Edge edges5[] = {
        {0, 1, 2}, {0, 3, 6},
        {1, 2, 3}, {1, 3, 8}, {1, 4, 5},
        {2, 4, 7},
        {3, 4, 9}
    };
    int num_e5 = sizeof(edges5) / sizeof(edges5[0]);
    CSRGraph *g5 = build_csr_from_edges(n5, num_e5, edges5, true);

    Edge edges5_copy[7];
    memcpy(edges5_copy, edges5, sizeof(edges5));

    long long cost_kruskal = run_kruskal(n5, num_e5, edges5_copy);
    long long cost_prim_arr = run_prim_array(n5, g5);
    long long cost_prim_heap = run_prim_heap(n5, g5);

    printf("Kruskal Total Cost   : %lld lakh\n", cost_kruskal);
    printf("Prim (Array) Total   : %lld lakh\n", cost_prim_arr);
    printf("Prim (Heap) Total    : %lld lakh\n", cost_prim_heap);
    printf("Status: %s\n", (cost_kruskal == 16 && cost_prim_arr == 16 && cost_prim_heap == 16)
                          ? "ALL MATCH EXPECTED 16 (VERIFIED)" : "MISMATCH");
    csr_free(g5);

    /* Part 2: Benchmarking Sparse vs Dense Graphs */
    printf("\n--- PART 2: Performance Scaling Benchmark (Median of 3 Runs) ---\n");
    printf("%-20s | %-8s | %-12s | %-16s | %-16s | %-16s\n",
           "Graph Type", "V", "E (undir)", "Prim Array (s)", "Prim Heap (s)", "Kruskal (s)");
    printf("----------------------------------------------------------------------------------------------------\n");

    /* Sparse Benchmark: V = 10,000, E ~ 30,000 */
    {
        int v_sparse = 10000;
        int target_e_sparse = 30000;
        srand(42);
        Edge *es = (Edge *)malloc(sizeof(Edge) * (size_t)target_e_sparse);
        for (int i = 1; i < v_sparse; i++) {
            es[i - 1] = (Edge){rand() % i, i, 1 + rand() % 100};
        }
        for (int i = v_sparse - 1; i < target_e_sparse; i++) {
            es[i] = (Edge){rand() % v_sparse, rand() % v_sparse, 1 + rand() % 100};
        }
        CSRGraph *g_sparse = build_csr_from_edges(v_sparse, target_e_sparse, es, true);

        MSTBenchArg b_arr = {v_sparse, target_e_sparse, es, g_sparse, 0};
        MSTBenchArg b_heap = {v_sparse, target_e_sparse, es, g_sparse, 1};
        MSTBenchArg b_krus = {v_sparse, target_e_sparse, es, g_sparse, 2};

        double t_arr = run_median_3(bench_mst, &b_arr);
        double t_heap = run_median_3(bench_mst, &b_heap);
        double t_krus = run_median_3(bench_mst, &b_krus);

        printf("%-20s | %-8d | %-12d | %-16.6f | %-16.6f | %-16.6f\n",
               "Sparse Network", v_sparse, target_e_sparse, t_arr, t_heap, t_krus);

        free(es);
        csr_free(g_sparse);
    }

    /* Dense Benchmark: V = 2,000, E ~ 1,000,000 */
    {
        int v_dense = 2000;
        int target_e_dense = 1000000;
        srand(42);
        Edge *ed = (Edge *)malloc(sizeof(Edge) * (size_t)target_e_dense);
        for (int i = 1; i < v_dense; i++) {
            ed[i - 1] = (Edge){rand() % i, i, 1 + rand() % 100};
        }
        for (int i = v_dense - 1; i < target_e_dense; i++) {
            ed[i] = (Edge){rand() % v_dense, rand() % v_dense, 1 + rand() % 100};
        }
        CSRGraph *g_dense = build_csr_from_edges(v_dense, target_e_dense, ed, true);

        MSTBenchArg b_arr = {v_dense, target_e_dense, ed, g_dense, 0};
        MSTBenchArg b_heap = {v_dense, target_e_dense, ed, g_dense, 1};
        MSTBenchArg b_krus = {v_dense, target_e_dense, ed, g_dense, 2};

        double t_arr = run_median_3(bench_mst, &b_arr);
        double t_heap = run_median_3(bench_mst, &b_heap);
        double t_krus = run_median_3(bench_mst, &b_krus);

        printf("%-20s | %-8d | %-12d | %-16.6f | %-16.6f | %-16.6f\n",
               "Dense Network", v_dense, target_e_dense, t_arr, t_heap, t_krus);

        free(ed);
        csr_free(g_dense);
    }

    printf("====================================================================================================\n");
    return 0;
}
