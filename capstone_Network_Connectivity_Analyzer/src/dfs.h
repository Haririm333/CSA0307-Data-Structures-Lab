#ifndef DFS_H
#define DFS_H

#include "graph.h"
#include <stdbool.h>

typedef struct DFSStep {
    int visit_id;
    int from_id; // -1 if root
    int* stack_snapshot;
    int stack_size;
} DFSStep;

typedef struct DFSResult {
    int* order;
    int order_count;
    int* levels;
    DFSStep* steps;
    int step_count;
} DFSResult;

DFSResult* dfs_traverse(const Graph* g, int start_id);
void dfs_result_free(DFSResult* res);
bool dfs_connected(const Graph* g, int u, int v);

#endif
