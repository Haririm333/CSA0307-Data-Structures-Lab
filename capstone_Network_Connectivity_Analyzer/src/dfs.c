#include "dfs.h"
#include <stdlib.h>
#include <string.h>

typedef struct StackFrame {
    int u;
    int from;
    int level;
} StackFrame;

DFSResult* dfs_traverse(const Graph* g, int start_id) {
    if (!g || start_id < 0 || start_id >= g->num_vertices || !g->vertices[start_id].active) {
        return NULL;
    }

    int n = g->num_vertices;
    DFSResult* res = (DFSResult*)malloc(sizeof(DFSResult));
    res->order = (int*)malloc(sizeof(int) * n);
    res->order_count = 0;
    res->levels = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) res->levels[i] = -1;

    res->steps = (DFSStep*)malloc(sizeof(DFSStep) * (n * 2 + 10));
    res->step_count = 0;

    bool* visited = (bool*)calloc(n, sizeof(bool));

    StackFrame* stack = (StackFrame*)malloc(sizeof(StackFrame) * (n * 4 + 20));
    int top = 0;

    // Push start
    stack[top++] = (StackFrame){ start_id, -1, 0 };

    while (top > 0) {
        StackFrame frame = stack[--top];
        int u = frame.u;

        if (visited[u]) continue;

        visited[u] = true;
        res->order[res->order_count++] = u;
        res->levels[u] = frame.level;

        // Collect unvisited active neighbors
        int n_count = 0;
        EdgeNode* edge = g->vertices[u].head;
        while (edge) {
            int v = edge->to;
            if (edge->active && g->vertices[v].active && !visited[v]) {
                n_count++;
            }
            edge = edge->next;
        }

        if (n_count > 0) {
            int* neighbors = (int*)malloc(sizeof(int) * n_count);
            int idx = 0;
            edge = g->vertices[u].head;
            while (edge) {
                int v = edge->to;
                if (edge->active && g->vertices[v].active && !visited[v]) {
                    neighbors[idx++] = v;
                }
                edge = edge->next;
            }
            // Push in reverse order so that alphabetically first neighbor is popped next
            for (int k = n_count - 1; k >= 0; k--) {
                stack[top++] = (StackFrame){ neighbors[k], u, frame.level + 1 };
            }
            free(neighbors);
        }

        // Snapshot stack
        DFSStep* step = &res->steps[res->step_count++];
        step->visit_id = u;
        step->from_id = frame.from;
        step->stack_size = top;
        if (top > 0) {
            step->stack_snapshot = (int*)malloc(sizeof(int) * top);
            for (int k = 0; k < top; k++) {
                step->stack_snapshot[k] = stack[k].u;
            }
        } else {
            step->stack_snapshot = NULL;
        }
    }

    free(visited);
    free(stack);
    return res;
}

void dfs_result_free(DFSResult* res) {
    if (!res) return;
    if (res->order) free(res->order);
    if (res->levels) free(res->levels);
    if (res->steps) {
        for (int i = 0; i < res->step_count; i++) {
            if (res->steps[i].stack_snapshot) {
                free(res->steps[i].stack_snapshot);
            }
        }
        free(res->steps);
    }
    free(res);
}

static bool dfs_reach(const Graph* g, int curr, int target, bool* visited) {
    if (curr == target) return true;
    visited[curr] = true;
    EdgeNode* edge = g->vertices[curr].head;
    while (edge) {
        int nxt = edge->to;
        if (edge->active && g->vertices[nxt].active && !visited[nxt]) {
            if (dfs_reach(g, nxt, target, visited)) return true;
        }
        edge = edge->next;
    }
    return false;
}

bool dfs_connected(const Graph* g, int u, int v) {
    if (!g || u < 0 || u >= g->num_vertices || v < 0 || v >= g->num_vertices) return false;
    if (!g->vertices[u].active || !g->vertices[v].active) return false;
    if (u == v) return true;

    bool* visited = (bool*)calloc(g->num_vertices, sizeof(bool));
    bool found = dfs_reach(g, u, v, visited);
    free(visited);
    return found;
}
