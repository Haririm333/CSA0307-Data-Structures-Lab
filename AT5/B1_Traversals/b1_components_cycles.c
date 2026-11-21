#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "graph.h"

int main(void) {
    printf("========================================================================================\n");
    printf("         CHENNAI MONSOON FLOODING: CONNECTED COMPONENTS & CYCLE DETECTION               \n");
    printf("========================================================================================\n\n");

    /* PART 1: 7-Node Chennai Metro Network */
    printf("--- PART 1: Chennai 7-Station Metro Network ---\n");
    Edge metro_edges[] = {
        {0, 1, 1}, {1, 2, 1}, {1, 5, 1}, {2, 3, 1},
        {3, 4, 1}, {4, 6, 1}, {5, 6, 1}
    };
    int num_metro = sizeof(metro_edges) / sizeof(metro_edges[0]);
    CSRGraph *g_metro = graph_create_csr(7, num_metro, metro_edges, true);

    int comp_metro_init = graph_count_components(g_metro, NULL);
    bool cycle_metro = graph_has_cycle_undirected(g_metro);

    printf("Original Metro Network:\n");
    printf(" - Connected Components: %d\n", comp_metro_init);
    printf(" - Has Cycle: %s (Loop exists via Guindy-Vadapalani-Koyambedu-Egmore-Central-Saidapet)\n",
           cycle_metro ? "YES" : "NO");

    /* Flooding scenario on Metro: sever connections to Koyambedu (edges connecting to station 6) */
    bool *flooded_metro = (bool *)calloc((size_t)g_metro->m, sizeof(bool));
    for (int u = 0; u < g_metro->n; u++) {
        for (int idx = g_metro->offset[u]; idx < g_metro->offset[u + 1]; idx++) {
            int v = g_metro->adj[idx];
            if ((u == 6 || v == 6) || (u == 1 && v == 5) || (u == 5 && v == 1)) {
                flooded_metro[idx] = true;
            }
        }
    }
    int comp_metro_flood = graph_count_components(g_metro, flooded_metro);
    printf("After Severe Flooding at Koyambedu & Vadapalani Junctions:\n");
    printf(" - Connected Components: %d (Network fractured into isolated islands)\n\n", comp_metro_flood);
    free(flooded_metro);
    graph_free_csr(g_metro);

    /* PART 2: 20-Node Chennai Road Network */
    printf("--- PART 2: 20-Junction Chennai Urban Arterial Network ---\n");
    printf("Covering major delivery corridors across Chennai: Central, Egmore, T.Nagar, Guindy,\n");
    printf("Adyar, Vadapalani, Koyambedu, Ambattur, Porur, Velachery, Tambaram, etc.\n");

    Edge road_edges[] = {
        {0, 1, 2}, {0, 9, 3}, {0, 14, 4}, {1, 2, 2}, {1, 13, 3},
        {2, 3, 3}, {2, 10, 4}, {3, 4, 2}, {3, 10, 3}, {4, 5, 2},
        {4, 6, 4}, {5, 6, 5}, {5, 17, 4}, {5, 18, 8}, {6, 7, 3},
        {7, 8, 2}, {8, 9, 2}, {10, 11, 3}, {10, 16, 5}, {11, 12, 3},
        {12, 13, 3}, {12, 15, 6}, {13, 0, 4}, {14, 13, 3}, {15, 19, 5},
        {16, 18, 9}, {17, 18, 7}
    };
    int num_roads = sizeof(road_edges) / sizeof(road_edges[0]);
    CSRGraph *g_roads = graph_create_csr(20, num_roads, road_edges, true);

    int comp_init = graph_count_components(g_roads, NULL);
    bool cycle_roads = graph_has_cycle_undirected(g_roads);

    printf("Original 20-Junction City Network:\n");
    printf(" - Connected Components: %d (Fully Connected)\n", comp_init);
    printf(" - Has Cycle: %s (Multiple alternative arterial routes)\n", cycle_roads ? "YES" : "NO");

    /* Flooding scenario: Adyar River & Cooum River burst banks */
    /* Sever Saidapet-Guindy (4-5), Adyar-Mylapore (6-7), Central-Triplicane (0-9) */
    bool *flooded_roads = (bool *)calloc((size_t)g_roads->m, sizeof(bool));
    for (int u = 0; u < g_roads->n; u++) {
        for (int idx = g_roads->offset[u]; idx < g_roads->offset[u + 1]; idx++) {
            int v = g_roads->adj[idx];
            if ((u == 4 && v == 5) || (u == 5 && v == 4) ||
                (u == 4 && v == 6) || (u == 6 && v == 4) ||
                (u == 5 && v == 6) || (u == 6 && v == 5) ||
                (u == 6 && v == 7) || (u == 7 && v == 6) ||
                (u == 0 && v == 9) || (u == 9 && v == 0) ||
                (u == 8 && v == 9) || (u == 9 && v == 8) ||
                (u == 16 && v == 18) || (u == 18 && v == 16) ||
                (u == 5 && v == 18) || (u == 18 && v == 5) ||
                (u == 17 && v == 18) || (u == 18 && v == 17)) {
                flooded_roads[idx] = true;
            }
        }
    }

    int comp_flooded = graph_count_components(g_roads, flooded_roads);
    printf("After Monsoon River Flooding (Adyar & Cooum River Bridges Submerged):\n");
    printf(" - Active Roads Cut: 9 critical river corridor bridges\n");
    printf(" - Connected Components: %d (South Chennai / Tambaram isolated from Central / North)\n\n", comp_flooded);

    free(flooded_roads);
    graph_free_csr(g_roads);

    /* PART 3: Cycle Detection Test on Pure Tree vs Cyclic Graph */
    printf("--- PART 3: Cycle Detection: Tree vs Cyclic Graph ---\n");
    /* Tree: 5 vertices, 4 edges, no loops */
    Edge tree_edges[] = {
        {0, 1, 1}, {0, 2, 1}, {1, 3, 1}, {1, 4, 1}
    };
    CSRGraph *g_tree = graph_create_csr(5, 4, tree_edges, true);
    printf("1. Hierarchical Feeder Tree (5 nodes, 4 edges):\n");
    printf("   - Has Cycle: %s (Expected: NO)\n", graph_has_cycle_undirected(g_tree) ? "YES" : "NO");
    graph_free_csr(g_tree);

    /* Cyclic: 5 vertices with a cross-edge forming a triangle */
    Edge cyclic_edges[] = {
        {0, 1, 1}, {0, 2, 1}, {1, 3, 1}, {1, 4, 1}, {3, 4, 1}
    };
    CSRGraph *g_cyclic = graph_create_csr(5, 5, cyclic_edges, true);
    printf("2. Ring Feeder Network (5 nodes, 5 edges with 3-4 loop):\n");
    printf("   - Has Cycle: %s (Expected: YES)\n", graph_has_cycle_undirected(g_cyclic) ? "YES" : "NO");
    graph_free_csr(g_cyclic);

    printf("========================================================================================\n");
    return 0;
}
