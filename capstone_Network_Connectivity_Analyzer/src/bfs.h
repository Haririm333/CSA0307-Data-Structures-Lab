#ifndef BFS_H
#define BFS_H

#include "graph.h"
#include <stdbool.h>

typedef struct BFSStep {
    int visit_id;
    int from_id; // -1 if root
    int* queue_snapshot;
    int queue_size;
} BFSStep;

typedef struct BFSResult {
    int* order;
    int order_count;
    int* levels; // indexed by vertex id, -1 if unreachable
    BFSStep* steps;
    int step_count;
} BFSResult;

BFSResult* bfs_traverse(const Graph* g, int start_id);
void bfs_result_free(BFSResult* res);
bool bfs_connected(const Graph* g, int u, int v);

#endif
