#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "sp_lib.h"

void dijkstra_array_single(const CSRGraph *g, int src, int *dist) {
    int n = g->n;
    bool *done = (bool *)calloc((size_t)n, sizeof(bool));
    if (!done) return;

    for (int i = 0; i < n; i++) dist[i] = INF_SP;
    dist[src] = 0;

    for (int step = 0; step < n; step++) {
        int u = -1;
        for (int v = 0; v < n; v++) {
            if (!done[v] && dist[v] != INF_SP && (u == -1 || dist[v] < dist[u])) {
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

void dijkstra_heap_single(const CSRGraph *g, int src, int *dist) {
    int n = g->n;
    for (int i = 0; i < n; i++) dist[i] = INF_SP;
    dist[src] = 0;

    MinHeap *heap = minheap_create(n * 2);
    if (!heap) return;
    minheap_push(heap, src, 0);

    while (!minheap_is_empty(heap)) {
        int u = 0;
        long long d = 0;
        if (!minheap_pop(heap, &u, &d)) break;

        /* Lazy deletion: ignore if a shorter path to u was already processed */
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

void dijkstra_array_all(const CSRGraph *g, int *all_dist) {
    int n = g->n;
    for (int i = 0; i < n; i++) {
        dijkstra_array_single(g, i, &all_dist[i * n]);
    }
}

void dijkstra_heap_all(const CSRGraph *g, int *all_dist) {
    int n = g->n;
    for (int i = 0; i < n; i++) {
        dijkstra_heap_single(g, i, &all_dist[i * n]);
    }
}

void floyd_warshall_flat(int n, const int *adj_matrix, int *dist) {
    memcpy(dist, adj_matrix, sizeof(int) * (size_t)(n * n));

    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            int dik = dist[i * n + k];
            /* INF guard: avoid unnecessary inner loop iterations and overflow */
            if (dik == INF_SP) continue;

            for (int j = 0; j < n; j++) {
                int dkj = dist[k * n + j];
                if (dkj == INF_SP) continue;

                if (dik + dkj < dist[i * n + j]) {
                    dist[i * n + j] = dik + dkj;
                }
            }
        }
    }
}

void csr_to_flat_matrix(const CSRGraph *g, int *mat) {
    int n = g->n;
    for (int i = 0; i < n * n; i++) mat[i] = INF_SP;
    for (int i = 0; i < n; i++) mat[i * n + i] = 0;

    for (int u = 0; u < n; u++) {
        for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
            int v = g->adj[idx];
            int w = g->weight[idx];
            if (w < mat[u * n + v]) {
                mat[u * n + v] = w;
            }
        }
    }
}

CSRGraph *generate_connected_graph_path(int n, int avg_deg, int min_w, int max_w, unsigned int seed) {
    srand(seed);
    if (n <= 1) return NULL;

    /* Guarantee connectivity by first creating a random Hamiltonian path */
    int *perm = (int *)malloc(sizeof(int) * (size_t)n);
    if (!perm) return NULL;
    for (int i = 0; i < n; i++) perm[i] = i;
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = perm[i];
        perm[i] = perm[j];
        perm[j] = tmp;
    }

    int target_edges = (n * avg_deg) / 2;
    if (target_edges < n - 1) target_edges = n - 1;

    Edge *edges = (Edge *)malloc(sizeof(Edge) * (size_t)target_edges);
    if (!edges) {
        free(perm);
        return NULL;
    }

    int edge_count = 0;
    for (int i = 0; i < n - 1; i++) {
        int u = perm[i];
        int v = perm[i + 1];
        int w = min_w + rand() % (max_w - min_w + 1);
        edges[edge_count++] = (Edge){u, v, w};
    }
    free(perm);

    while (edge_count < target_edges) {
        int u = rand() % n;
        int v = rand() % n;
        if (u != v) {
            int w = min_w + rand() % (max_w - min_w + 1);
            edges[edge_count++] = (Edge){u, v, w};
        }
    }

    CSRGraph *g = build_csr_from_edges(n, edge_count, edges, true);
    free(edges);
    return g;
}
