#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../src/graph.h"
#include "../src/bfs.h"
#include "../src/dfs.h"
#include "../src/unionfind.h"
#include "../src/dijkstra.h"
#include "../src/faults.h"

static int tests_run = 0;
static int tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    tests_run++; \
    if (cond) { \
        tests_passed++; \
    } else { \
        printf("FAILED [line %d]: %s\n", __LINE__, msg); \
    } \
} while (0)

void test_graph_basic(void) {
    Graph* g = graph_create(4);
    TEST_ASSERT(g != NULL, "Graph created");

    int id_a = graph_add_vertex(g, "A");
    int id_b = graph_add_vertex(g, "B");
    TEST_ASSERT(id_a == 0 && id_b == 1, "Vertices added with correct IDs");
    TEST_ASSERT(graph_find_vertex(g, "A") == 0, "Find vertex A");
    TEST_ASSERT(graph_find_vertex(g, "Unknown") == -1, "Find unknown vertex returns -1");

    bool added = graph_add_edge(g, "A", "B", 10);
    TEST_ASSERT(added, "Edge A-B added");
    TEST_ASSERT(graph_has_edge(g, id_a, id_b), "Graph has edge A-B");
    TEST_ASSERT(graph_has_edge(g, id_b, id_a), "Graph has edge B-A (undirected)");
    TEST_ASSERT(g->vertices[id_a].degree == 1, "Vertex A degree is 1");
    TEST_ASSERT(g->vertices[id_b].degree == 1, "Vertex B degree is 1");

    // Self loop rejected
    TEST_ASSERT(!graph_add_edge(g, "A", "A", 5), "Self-loop rejected");

    graph_free(g);
}

void test_sample_small_bridges(void) {
    Graph* g = graph_load_from_file("data/sample_small.txt");
    TEST_ASSERT(g != NULL, "Loaded sample_small.txt");
    if (!g) return;

    TEST_ASSERT(g->num_vertices == 4, "sample_small has 4 vertices");

    FaultResult* fres = faults_detect(g);
    TEST_ASSERT(fres != NULL, "faults_detect executed");
    if (fres) {
        TEST_ASSERT(fres->bridge_count == 3, "sample_small has exactly 3 bridges");
        TEST_ASSERT(fres->articulation_count == 2, "sample_small has 2 articulation points");
        fault_result_free(fres);
    }
    graph_free(g);
}

void test_sample_dijkstra(void) {
    Graph* g = graph_load_from_file("data/sample_dijkstra.txt");
    TEST_ASSERT(g != NULL, "Loaded sample_dijkstra.txt");
    if (!g) return;

    int src = graph_find_vertex(g, "A");
    int dst = graph_find_vertex(g, "D");
    TEST_ASSERT(src >= 0 && dst >= 0, "A and D found in sample_dijkstra");

    DijkstraResult* dres = dijkstra_shortest_path(g, src, dst);
    TEST_ASSERT(dres != NULL, "Dijkstra ran successfully");
    if (dres) {
        TEST_ASSERT(dres->reachable == true, "A to D is reachable");
        TEST_ASSERT(dres->total_cost == 5, "Shortest path cost is 5");
        TEST_ASSERT(dres->path_len == 3, "Path length is 3 nodes (A, B, D)");
        if (dres->path_len == 3) {
            TEST_ASSERT(strcmp(g->vertices[dres->path[0]].name, "A") == 0, "Path[0] is A");
            TEST_ASSERT(strcmp(g->vertices[dres->path[1]].name, "B") == 0, "Path[1] is B");
            TEST_ASSERT(strcmp(g->vertices[dres->path[2]].name, "D") == 0, "Path[2] is D");
        }
        dijkstra_result_free(dres);
    }
    graph_free(g);
}

void test_sample_disconnected(void) {
    Graph* g = graph_load_from_file("data/sample_disconnected.txt");
    TEST_ASSERT(g != NULL, "Loaded sample_disconnected.txt");
    if (!g) return;

    ComponentList* comps = uf_get_components(g);
    TEST_ASSERT(comps != NULL, "uf_get_components succeeded");
    if (comps) {
        TEST_ASSERT(comps->count >= 2, "sample_disconnected has 2 or more components");
        component_list_free(comps);
    }

    int u = graph_find_vertex(g, "A");
    int v = graph_find_vertex(g, "D");
    int iso = graph_find_vertex(g, "F");

    TEST_ASSERT(bfs_connected(g, u, v) == false, "A and D not connected in disconnected sample");
    TEST_ASSERT(dfs_connected(g, u, v) == false, "DFS agrees A and D not connected");
    TEST_ASSERT(g->vertices[iso].degree == 0, "Device F is isolated (degree 0)");

    graph_free(g);
}

void test_simulation(void) {
    Graph* g = graph_load_from_file("data/sample_small.txt");
    TEST_ASSERT(g != NULL, "Loaded sample_small for simulation test");
    if (!g) return;

    SimResult* sres = faults_simulate_link(g, "A", "B");
    TEST_ASSERT(sres != NULL, "Simulate link A-B ran");
    if (sres) {
        TEST_ASSERT(sres->components_before == 1, "Components before was 1");
        TEST_ASSERT(sres->components_after == 2, "Components after is 2");
        sim_result_free(sres);
    }

    SimResult* dres = faults_simulate_device(g, "B");
    TEST_ASSERT(dres != NULL, "Simulate device B ran");
    if (dres) {
        TEST_ASSERT(dres->components_before == 1, "Components before was 1");
        TEST_ASSERT(dres->components_after == 2, "Components after is 2");
        sim_result_free(dres);
    }
    graph_free(g);
}

int main(void) {
    printf("=========================================\n");
    printf("  RUNNING C CORE UNIT TESTS             \n");
    printf("=========================================\n");

    test_graph_basic();
    test_sample_small_bridges();
    test_sample_dijkstra();
    test_sample_disconnected();
    test_simulation();

    printf("\nC Unit Tests: %d passed / %d run\n", tests_passed, tests_run);
    if (tests_passed == tests_run) {
        printf("ALL C UNIT TESTS PASSED SUCCESSFULLY!\n");
        return 0;
    } else {
        printf("SOME C TESTS FAILED!\n");
        return 1;
    }
}
