#ifndef GRAPH_GEN_H
#define GRAPH_GEN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    int n;          /* Number of vertices */
    int m;          /* Number of directed edges */
    int *offset;    /* Size n + 1: start indices in adj/weight */
    int *adj;       /* Size m: target vertices */
    int *weight;    /* Size m: edge weights (NULL if unweighted) */
} CSRGraph;

typedef struct {
    int u;
    int v;
    int w;
} Edge;

static inline size_t csr_analytic_bytes(int n, int m, bool has_weight) {
    size_t total = sizeof(CSRGraph);
    total += (size_t)(n + 1) * sizeof(int);
    total += (size_t)m * sizeof(int);
    if (has_weight) {
        total += (size_t)m * sizeof(int);
    }
    return total;
}

static inline void csr_free(CSRGraph *g) {
    if (g) {
        free(g->offset);
        free(g->adj);
        if (g->weight) free(g->weight);
        free(g);
    }
}

/* Compare edges by source vertex u, then target vertex v */
static inline int compare_edges(const void *a, const void *b) {
    const Edge *ea = (const Edge *)a;
    const Edge *eb = (const Edge *)b;
    if (ea->u != eb->u) return ea->u - eb->u;
    return ea->v - eb->v;
}

/* Build CSR from an array of edges */
static inline CSRGraph *build_csr_from_edges(int n, int num_edges, const Edge *edges, bool undirected) {
    CSRGraph *g = (CSRGraph *)malloc(sizeof(CSRGraph));
    if (!g) return NULL;
    g->n = n;

    int total_edges = undirected ? num_edges * 2 : num_edges;
    Edge *all_edges = (Edge *)malloc(sizeof(Edge) * (size_t)total_edges);
    if (!all_edges) {
        free(g);
        return NULL;
    }

    int idx = 0;
    for (int i = 0; i < num_edges; i++) {
        all_edges[idx++] = edges[i];
        if (undirected) {
            all_edges[idx].u = edges[i].v;
            all_edges[idx].v = edges[i].u;
            all_edges[idx].w = edges[i].w;
            idx++;
        }
    }

    qsort(all_edges, (size_t)total_edges, sizeof(Edge), compare_edges);

    /* Allocate CSR arrays */
    g->offset = (int *)calloc((size_t)(n + 1), sizeof(int));
    g->adj = (int *)malloc(sizeof(int) * (size_t)total_edges);
    g->weight = (int *)malloc(sizeof(int) * (size_t)total_edges);

    if (!g->offset || !g->adj || !g->weight) {
        free(all_edges);
        csr_free(g);
        return NULL;
    }

    g->m = total_edges;
    /* Count degrees */
    for (int i = 0; i < total_edges; i++) {
        int u = all_edges[i].u;
        if (u >= 0 && u < n) {
            g->offset[u + 1]++;
        }
    }
    /* Cumulative sum */
    for (int i = 0; i < n; i++) {
        g->offset[i + 1] += g->offset[i];
    }

    int *cur_pos = (int *)malloc(sizeof(int) * (size_t)n);
    if (!cur_pos) {
        free(all_edges);
        csr_free(g);
        return NULL;
    }
    memcpy(cur_pos, g->offset, sizeof(int) * (size_t)n);

    for (int i = 0; i < total_edges; i++) {
        int u = all_edges[i].u;
        if (u >= 0 && u < n) {
            int p = cur_pos[u]++;
            g->adj[p] = all_edges[i].v;
            g->weight[p] = all_edges[i].w;
        }
    }

    free(cur_pos);
    free(all_edges);
    return g;
}

/* Generate a random connected graph with ~avg_deg edges per vertex */
static inline CSRGraph *generate_random_connected_csr(int n, int avg_deg, int min_w, int max_w, unsigned int seed) {
    srand(seed);
    if (n <= 1) return NULL;

    /* First, create a spanning tree: n-1 edges */
    int target_edges = (n * avg_deg) / 2;
    if (target_edges < n - 1) target_edges = n - 1;

    Edge *edges = (Edge *)malloc(sizeof(Edge) * (size_t)target_edges);
    if (!edges) return NULL;

    int edge_count = 0;

    /* Spanning tree (undirected) to ensure connectivity */
    for (int i = 1; i < n; i++) {
        int parent = rand() % i;
        int w = min_w + rand() % (max_w - min_w + 1);
        edges[edge_count++] = (Edge){parent, i, w};
    }

    /* Add remaining random edges */
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

#endif /* GRAPH_GEN_H */
