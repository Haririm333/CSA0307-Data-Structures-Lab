#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"
#include <stdbool.h>

#define INF_DIST 1000000000

typedef struct DijkstraStep {
    int settled_id;
    int* dist_snapshot; // dist to each vertex at this moment (-1 or INF if not yet discovered)
} DijkstraStep;

typedef struct PathEdge {
    int u;
    int v;
    int cost;
} PathEdge;

typedef struct DijkstraResult {
    bool reachable;
    int total_cost;
    int* path;
    int path_len;
    PathEdge* edges;
    int edge_count;
    DijkstraStep* steps;
    int step_count;
} DijkstraResult;

DijkstraResult* dijkstra_shortest_path(const Graph* g, int src_id, int dst_id);
void dijkstra_result_free(DijkstraResult* res);

#endif
