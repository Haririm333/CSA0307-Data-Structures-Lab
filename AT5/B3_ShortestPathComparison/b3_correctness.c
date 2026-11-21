#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "sp_lib.h"

int main(void) {
    printf("========================================================================================\n");
    printf("        B3.1: CORRECTNESS VERIFICATION OF SHORTEST-PATH ALGORITHMS (8 NODES)            \n");
    printf("========================================================================================\n\n");

    int n = 8;
    CSRGraph *g = generate_connected_graph_path(n, 3, 5, 50, 42);
    if (!g) {
        fprintf(stderr, "Failed to generate graph\n");
        return 1;
    }

    printf("Generated 8-Vertex Connected Graph (Weights 5 to 50):\n");
    printf("Edge List (Undirected):\n");
    for (int u = 0; u < n; u++) {
        for (int idx = g->offset[u]; idx < g->offset[u + 1]; idx++) {
            int v = g->adj[idx];
            if (u < v) {
                printf("  (%d - %d, weight = %d)\n", u, v, g->weight[idx]);
            }
        }
    }
    printf("\n");

    int *mat = (int *)malloc(sizeof(int) * (size_t)(n * n));
    csr_to_flat_matrix(g, mat);

    int *dist_fw = (int *)malloc(sizeof(int) * (size_t)(n * n));
    int *dist_dij_arr = (int *)malloc(sizeof(int) * (size_t)(n * n));
    int *dist_dij_heap = (int *)malloc(sizeof(int) * (size_t)(n * n));

    floyd_warshall_flat(n, mat, dist_fw);
    dijkstra_array_all(g, dist_dij_arr);
    dijkstra_heap_all(g, dist_dij_heap);

    /* Print distance tables side by side */
    printf("--- All-Pairs Distance Tables (Rows 0 to %d) ---\n", n - 1);
    for (int i = 0; i < n; i++) {
        printf("Source %d:\n", i);
        printf("  Floyd-Warshall  : [");
        for (int j = 0; j < n; j++) printf(" %3d", dist_fw[i * n + j]);
        printf(" ]\n");

        printf("  Dijkstra (Array): [");
        for (int j = 0; j < n; j++) printf(" %3d", dist_dij_arr[i * n + j]);
        printf(" ]\n");

        printf("  Dijkstra (Heap) : [");
        for (int j = 0; j < n; j++) printf(" %3d", dist_dij_heap[i * n + j]);
        printf(" ]\n\n");
    }

    /* Programmatic verification */
    bool all_match = true;
    int mismatches = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int idx = i * n + j;
            if (dist_fw[idx] != dist_dij_arr[idx] || dist_fw[idx] != dist_dij_heap[idx]) {
                all_match = false;
                mismatches++;
                printf("MISMATCH at (%d, %d): FW=%d, Arr=%d, Heap=%d\n",
                       i, j, dist_fw[idx], dist_dij_arr[idx], dist_dij_heap[idx]);
            }
        }
    }

    printf("========================================================================================\n");
    if (all_match && mismatches == 0) {
        printf("VERIFICATION RESULT: ALL THREE MATCH\n");
        printf("Every all-pairs distance entry across all 64 pairs is identical across all 3 algorithms.\n");
    } else {
        printf("VERIFICATION RESULT: FAILED (%d mismatches)\n", mismatches);
    }
    printf("========================================================================================\n");

    free(mat);
    free(dist_fw);
    free(dist_dij_arr);
    free(dist_dij_heap);
    csr_free(g);

    return all_match ? 0 : 1;
}
