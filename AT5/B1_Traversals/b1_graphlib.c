#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "graph.h"

CSRGraph *graph_create_csr(int n, int num_edges, const Edge *edges, bool undirected) {
    return build_csr_from_edges(n, num_edges, edges, undirected);
}

void graph_free_csr(CSRGraph *g) {
    csr_free(g);
}

void graph_bfs(const CSRGraph *g, int src, int *dist, int *parent) {
    int n = g->n;
    for (int i = 0; i < n; i++) {
        dist[i] = -1;
        parent[i] = -1;
    }
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
                parent[v] = u;
                queue[tail++] = v;
            }
        }
    }

    free(queue);
}

static void dfs_rec_helper(const CSRGraph *g, int u, bool *seen, int *order, int *count) {
    seen[u] = true;
    order[(*count)++] = u;
    for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
        int v = g->adj[idx];
        if (!seen[v]) {
            dfs_rec_helper(g, v, seen, order, count);
        }
    }
}

void graph_dfs_recursive(const CSRGraph *g, int src, int *visited_order, int *count) {
    int n = g->n;
    bool *seen = (bool *)calloc((size_t)n, sizeof(bool));
    if (!seen) return;
    *count = 0;
    dfs_rec_helper(g, src, seen, visited_order, count);
    free(seen);
}

typedef struct {
    int u;
    int edge_idx;
} StackItem;

void graph_dfs_iterative(const CSRGraph *g, int src, int *visited_order, int *count) {
    int n = g->n;
    bool *seen = (bool *)calloc((size_t)n, sizeof(bool));
    StackItem *stack = (StackItem *)malloc(sizeof(StackItem) * (size_t)n);
    if (!seen || !stack) {
        free(seen);
        free(stack);
        return;
    }

    int top = 0;
    *count = 0;

    seen[src] = true;
    visited_order[(*count)++] = src;
    stack[top] = (StackItem){src, g->offset[src]};

    while (top >= 0) {
        StackItem *cur = &stack[top];
        int u = cur->u;
        bool advanced = false;

        while (cur->edge_idx < g->offset[u + 1]) {
            int v = g->adj[cur->edge_idx++];
            if (!seen[v]) {
                seen[v] = true;
                visited_order[(*count)++] = v;
                top++;
                stack[top] = (StackItem){v, g->offset[v]};
                advanced = true;
                break;
            }
        }

        if (!advanced) {
            top--;
        }
    }

    free(seen);
    free(stack);
}

int graph_count_components(const CSRGraph *g, const bool *edge_disabled) {
    int n = g->n;
    bool *visited = (bool *)calloc((size_t)n, sizeof(bool));
    int *queue = (int *)malloc(sizeof(int) * (size_t)n);
    if (!visited || !queue) {
        free(visited);
        free(queue);
        return 0;
    }

    int components = 0;
    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            components++;
            int head = 0, tail = 0;
            visited[i] = true;
            queue[tail++] = i;

            while (head < tail) {
                int u = queue[head++];
                for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
                    if (edge_disabled && edge_disabled[idx]) continue;
                    int v = g->adj[idx];
                    if (!visited[v]) {
                        visited[v] = true;
                        queue[tail++] = v;
                    }
                }
            }
        }
    }

    free(visited);
    free(queue);
    return components;
}

static bool cycle_dfs(const CSRGraph *g, int u, int parent, bool *visited) {
    visited[u] = true;
    for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
        int v = g->adj[idx];
        if (!visited[v]) {
            if (cycle_dfs(g, v, u, visited)) return true;
        } else if (v != parent) {
            return true; /* Cycle detected */
        }
    }
    return false;
}

bool graph_has_cycle_undirected(const CSRGraph *g) {
    int n = g->n;
    bool *visited = (bool *)calloc((size_t)n, sizeof(bool));
    if (!visited) return false;

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            if (cycle_dfs(g, i, -1, visited)) {
                free(visited);
                return true;
            }
        }
    }

    free(visited);
    return false;
}
