#ifndef SP_LIB_H
#define SP_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../common/timer.h"
#include "../common/minheap.h"
#include "../common/graph_gen.h"

#define INF_SP 999999999

/* Single-source shortest path */
void dijkstra_array_single(const CSRGraph *g, int src, int *dist);
void dijkstra_heap_single(const CSRGraph *g, int src, int *dist);

/* All-pairs shortest path */
void dijkstra_array_all(const CSRGraph *g, int *all_dist);
void dijkstra_heap_all(const CSRGraph *g, int *all_dist);
void floyd_warshall_flat(int n, const int *adj_matrix, int *dist);

/* Helper to convert CSR graph to flat adjacency matrix */
void csr_to_flat_matrix(const CSRGraph *g, int *mat);

/* Graph generation helper with spanning path for guaranteed connectivity */
CSRGraph *generate_connected_graph_path(int n, int avg_deg, int min_w, int max_w, unsigned int seed);

#endif /* SP_LIB_H */
