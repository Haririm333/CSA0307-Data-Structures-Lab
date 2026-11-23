#include "dijkstra.h"
#include <stdlib.h>
#include <string.h>

typedef struct HeapNode {
    int u;
    int dist;
} HeapNode;

typedef struct MinHeap {
    HeapNode* data;
    int size;
    int capacity;
} MinHeap;

static MinHeap* heap_create(int capacity) {
    MinHeap* h = (MinHeap*)malloc(sizeof(MinHeap));
    h->size = 0;
    h->capacity = capacity;
    h->data = (HeapNode*)malloc(sizeof(HeapNode) * capacity);
    return h;
}

static void heap_free(MinHeap* h) {
    if (!h) return;
    free(h->data);
    free(h);
}

static void heap_push(MinHeap* h, int u, int dist) {
    if (h->size >= h->capacity) {
        h->capacity *= 2;
        h->data = (HeapNode*)realloc(h->data, sizeof(HeapNode) * h->capacity);
    }
    int idx = h->size++;
    h->data[idx] = (HeapNode){ u, dist };
    // Bubble up
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (h->data[parent].dist > h->data[idx].dist) {
            HeapNode tmp = h->data[parent];
            h->data[parent] = h->data[idx];
            h->data[idx] = tmp;
            idx = parent;
        } else {
            break;
        }
    }
}

static HeapNode heap_pop(MinHeap* h) {
    HeapNode min_node = h->data[0];
    h->data[0] = h->data[--h->size];
    // Sift down
    int idx = 0;
    while (true) {
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        int smallest = idx;
        if (left < h->size && h->data[left].dist < h->data[smallest].dist) smallest = left;
        if (right < h->size && h->data[right].dist < h->data[smallest].dist) smallest = right;
        if (smallest != idx) {
            HeapNode tmp = h->data[idx];
            h->data[idx] = h->data[smallest];
            h->data[smallest] = tmp;
            idx = smallest;
        } else {
            break;
        }
    }
    return min_node;
}

DijkstraResult* dijkstra_shortest_path(const Graph* g, int src_id, int dst_id) {
    if (!g || src_id < 0 || src_id >= g->num_vertices || dst_id < 0 || dst_id >= g->num_vertices) {
        return NULL;
    }
    if (!g->vertices[src_id].active || !g->vertices[dst_id].active) {
        return NULL;
    }

    int n = g->num_vertices;
    DijkstraResult* res = (DijkstraResult*)malloc(sizeof(DijkstraResult));
    res->reachable = false;
    res->total_cost = -1;
    res->path = NULL;
    res->path_len = 0;
    res->edges = NULL;
    res->edge_count = 0;
    res->steps = (DijkstraStep*)malloc(sizeof(DijkstraStep) * (n * 2 + 10));
    res->step_count = 0;

    int* dist = (int*)malloc(sizeof(int) * n);
    int* parent = (int*)malloc(sizeof(int) * n);
    bool* settled = (bool*)calloc(n, sizeof(bool));

    for (int i = 0; i < n; i++) {
        dist[i] = INF_DIST;
        parent[i] = -1;
    }

    MinHeap* pq = heap_create(n * 2 + 16);
    dist[src_id] = 0;
    heap_push(pq, src_id, 0);

    while (pq->size > 0) {
        HeapNode top = heap_pop(pq);
        int u = top.u;
        if (settled[u]) continue;
        settled[u] = true;

        // Relax neighbors
        EdgeNode* edge = g->vertices[u].head;
        while (edge) {
            int v = edge->to;
            if (edge->active && g->vertices[v].active && !settled[v]) {
                int new_dist = dist[u] + edge->cost;
                if (new_dist < dist[v]) {
                    dist[v] = new_dist;
                    parent[v] = u;
                    heap_push(pq, v, new_dist);
                }
            }
            edge = edge->next;
        }

        // Record step snapshot
        DijkstraStep* step = &res->steps[res->step_count++];
        step->settled_id = u;
        step->dist_snapshot = (int*)malloc(sizeof(int) * n);
        for (int i = 0; i < n; i++) {
            step->dist_snapshot[i] = dist[i];
        }

        if (u == dst_id) {
            break;
        }
    }

    if (dist[dst_id] < INF_DIST) {
        res->reachable = true;
        res->total_cost = dist[dst_id];

        // Reconstruct path
        int curr = dst_id;
        int len = 0;
        while (curr != -1) {
            len++;
            curr = parent[curr];
        }

        res->path_len = len;
        res->path = (int*)malloc(sizeof(int) * len);
        curr = dst_id;
        for (int k = len - 1; k >= 0; k--) {
            res->path[k] = curr;
            curr = parent[curr];
        }

        // Build edges
        if (len > 1) {
            res->edge_count = len - 1;
            res->edges = (PathEdge*)malloc(sizeof(PathEdge) * res->edge_count);
            for (int k = 0; k < len - 1; k++) {
                int u = res->path[k];
                int v = res->path[k + 1];
                int cost = 0;
                EdgeNode* edge = g->vertices[u].head;
                while (edge) {
                    if (edge->to == v) { cost = edge->cost; break; }
                    edge = edge->next;
                }
                res->edges[k] = (PathEdge){ u, v, cost };
            }
        }
    }

    heap_free(pq);
    free(dist);
    free(parent);
    free(settled);
    return res;
}

void dijkstra_result_free(DijkstraResult* res) {
    if (!res) return;
    if (res->path) free(res->path);
    if (res->edges) free(res->edges);
    if (res->steps) {
        for (int i = 0; i < res->step_count; i++) {
            if (res->steps[i].dist_snapshot) {
                free(res->steps[i].dist_snapshot);
            }
        }
        free(res->steps);
    }
    free(res);
}
