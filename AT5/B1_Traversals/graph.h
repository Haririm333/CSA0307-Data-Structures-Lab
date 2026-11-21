#ifndef B1_GRAPH_H
#define B1_GRAPH_H

#include <stdbool.h>
#include "../common/graph_gen.h"

CSRGraph *graph_create_csr(int n, int num_edges, const Edge *edges, bool undirected);
void graph_free_csr(CSRGraph *g);
void graph_bfs(const CSRGraph *g, int src, int *dist, int *parent);
void graph_dfs_recursive(const CSRGraph *g, int src, int *visited_order, int *count);
void graph_dfs_iterative(const CSRGraph *g, int src, int *visited_order, int *count);
int graph_count_components(const CSRGraph *g, const bool *edge_disabled);
bool graph_has_cycle_undirected(const CSRGraph *g);

#endif /* B1_GRAPH_H */
