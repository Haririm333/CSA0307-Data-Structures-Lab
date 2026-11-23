#include "bfs.h"
#include <stdlib.h>
#include <string.h>

BFSResult* bfs_traverse(const Graph* g, int start_id) {
    if (!g || start_id < 0 || start_id >= g->num_vertices || !g->vertices[start_id].active) {
        return NULL;
    }

    int n = g->num_vertices;
    BFSResult* res = (BFSResult*)malloc(sizeof(BFSResult));
    res->order = (int*)malloc(sizeof(int) * n);
    res->order_count = 0;
    res->levels = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) res->levels[i] = -1;

    res->steps = (BFSStep*)malloc(sizeof(BFSStep) * n);
    res->step_count = 0;

    bool* visited = (bool*)calloc(n, sizeof(bool));
    int* from = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) from[i] = -1;

    // Queue implementation
    int* queue = (int*)malloc(sizeof(int) * (n * 2 + 10));
    int q_head = 0, q_tail = 0;

    // Enqueue start
    queue[q_tail++] = start_id;
    visited[start_id] = true;
    res->levels[start_id] = 0;

    while (q_head < q_tail) {
        int u = queue[q_head++];
        res->order[res->order_count++] = u;

        // Explore neighbors
        EdgeNode* edge = g->vertices[u].head;
        while (edge) {
            int v = edge->to;
            if (edge->active && g->vertices[v].active && !visited[v]) {
                visited[v] = true;
                from[v] = u;
                res->levels[v] = res->levels[u] + 1;
                queue[q_tail++] = v;
            }
            edge = edge->next;
        }

        // Snapshot remaining elements in queue
        int rem_count = q_tail - q_head;
        BFSStep* step = &res->steps[res->step_count++];
        step->visit_id = u;
        step->from_id = from[u];
        step->queue_size = rem_count;
        if (rem_count > 0) {
            step->queue_snapshot = (int*)malloc(sizeof(int) * rem_count);
            for (int k = 0; k < rem_count; k++) {
                step->queue_snapshot[k] = queue[q_head + k];
            }
        } else {
            step->queue_snapshot = NULL;
        }
    }

    free(visited);
    free(from);
    free(queue);
    return res;
}

void bfs_result_free(BFSResult* res) {
    if (!res) return;
    if (res->order) free(res->order);
    if (res->levels) free(res->levels);
    if (res->steps) {
        for (int i = 0; i < res->step_count; i++) {
            if (res->steps[i].queue_snapshot) {
                free(res->steps[i].queue_snapshot);
            }
        }
        free(res->steps);
    }
    free(res);
}

bool bfs_connected(const Graph* g, int u, int v) {
    if (!g || u < 0 || u >= g->num_vertices || v < 0 || v >= g->num_vertices) return false;
    if (!g->vertices[u].active || !g->vertices[v].active) return false;
    if (u == v) return true;

    int n = g->num_vertices;
    bool* visited = (bool*)calloc(n, sizeof(bool));
    int* queue = (int*)malloc(sizeof(int) * (n + 5));
    int q_head = 0, q_tail = 0;

    queue[q_tail++] = u;
    visited[u] = true;
    bool found = false;

    while (q_head < q_tail) {
        int curr = queue[q_head++];
        if (curr == v) {
            found = true;
            break;
        }
        EdgeNode* edge = g->vertices[curr].head;
        while (edge) {
            int nxt = edge->to;
            if (edge->active && g->vertices[nxt].active && !visited[nxt]) {
                visited[nxt] = true;
                queue[q_tail++] = nxt;
            }
            edge = edge->next;
        }
    }

    free(visited);
    free(queue);
    return found;
}
