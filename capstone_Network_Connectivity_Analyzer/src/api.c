#include "api.h"
#include "graph.h"
#include "bfs.h"
#include "dfs.h"
#include "unionfind.h"
#include "dijkstra.h"
#include "faults.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_json_string(const char* s) {
    putchar('"');
    if (s) {
        for (const char* p = s; *p; p++) {
            switch (*p) {
                case '\\': printf("\\\\"); break;
                case '"':  printf("\\\""); break;
                case '\b': printf("\\b"); break;
                case '\f': printf("\\f"); break;
                case '\n': printf("\\n"); break;
                case '\r': printf("\\r"); break;
                case '\t': printf("\\t"); break;
                default:
                    if ((unsigned char)*p < 0x20) {
                        printf("\\u%04x", (unsigned char)*p);
                    } else {
                        putchar(*p);
                    }
                    break;
            }
        }
    }
    putchar('"');
}

static void print_error_and_exit(const char* msg) {
    printf("{\"ok\":false,\"error\":");
    print_json_string(msg);
    printf("}\n");
    exit(1);
}

static void cmd_summary(const Graph* g) {
    printf("{\"ok\":true,\"devices\":[");
    for (int i = 0; i < g->num_vertices; i++) {
        if (i > 0) printf(",");
        printf("{\"id\":%d,\"name\":", g->vertices[i].id);
        print_json_string(g->vertices[i].name);
        printf(",\"degree\":%d}", g->vertices[i].degree);
    }
    printf("],\"links\":[");

    int link_count = 0;
    for (int i = 0; i < g->num_vertices; i++) {
        EdgeNode* edge = g->vertices[i].head;
        while (edge) {
            int to = edge->to;
            if (i < to) {
                if (link_count > 0) printf(",");
                printf("{\"a\":");
                print_json_string(g->vertices[i].name);
                printf(",\"b\":");
                print_json_string(g->vertices[to].name);
                printf(",\"cost\":%d}", edge->cost);
                link_count++;
            }
            edge = edge->next;
        }
    }
    printf("],");

    ComponentList* comps = uf_get_components(g);
    int total_comps = comps ? comps->count : 0;
    printf("\"components\":%d,", total_comps);

    printf("\"isolated\":[");
    int iso_count = 0;
    for (int i = 0; i < g->num_vertices; i++) {
        if (g->vertices[i].degree == 0) {
            if (iso_count > 0) printf(",");
            print_json_string(g->vertices[i].name);
            iso_count++;
        }
    }
    printf("],\"componentList\":[");

    if (comps) {
        for (int k = 0; k < comps->count; k++) {
            if (k > 0) printf(",");
            printf("[");
            for (int j = 0; j < comps->groups[k].size; j++) {
                if (j > 0) printf(",");
                int vid = comps->groups[k].vertex_ids[j];
                print_json_string(g->vertices[vid].name);
            }
            printf("]");
        }
        component_list_free(comps);
    }
    printf("]}\n");
}

static void cmd_bfs(const Graph* g, const char* start_name) {
    int start_id = graph_find_vertex(g, start_name);
    if (start_id < 0) {
        print_error_and_exit("Start device not found");
    }

    BFSResult* res = bfs_traverse(g, start_id);
    if (!res) {
        print_error_and_exit("Failed to run BFS");
    }

    printf("{\"ok\":true,\"order\":[");
    for (int i = 0; i < res->order_count; i++) {
        if (i > 0) printf(",");
        print_json_string(g->vertices[res->order[i]].name);
    }
    printf("],\"levels\":{");
    int lvl_count = 0;
    for (int i = 0; i < res->order_count; i++) {
        int u = res->order[i];
        if (res->levels[u] >= 0) {
            if (lvl_count > 0) printf(",");
            print_json_string(g->vertices[u].name);
            printf(":%d", res->levels[u]);
            lvl_count++;
        }
    }
    printf("},\"steps\":[");
    for (int i = 0; i < res->step_count; i++) {
        if (i > 0) printf(",");
        BFSStep* step = &res->steps[i];
        printf("{\"visit\":");
        print_json_string(g->vertices[step->visit_id].name);
        printf(",\"from\":");
        if (step->from_id >= 0) {
            print_json_string(g->vertices[step->from_id].name);
        } else {
            printf("null");
        }
        printf(",\"queue\":[");
        for (int k = 0; k < step->queue_size; k++) {
            if (k > 0) printf(",");
            print_json_string(g->vertices[step->queue_snapshot[k]].name);
        }
        printf("]}");
    }
    printf("]}\n");
    bfs_result_free(res);
}

static void cmd_dfs(const Graph* g, const char* start_name) {
    int start_id = graph_find_vertex(g, start_name);
    if (start_id < 0) {
        print_error_and_exit("Start device not found");
    }

    DFSResult* res = dfs_traverse(g, start_id);
    if (!res) {
        print_error_and_exit("Failed to run DFS");
    }

    printf("{\"ok\":true,\"order\":[");
    for (int i = 0; i < res->order_count; i++) {
        if (i > 0) printf(",");
        print_json_string(g->vertices[res->order[i]].name);
    }
    printf("],\"levels\":{");
    int lvl_count = 0;
    for (int i = 0; i < res->order_count; i++) {
        int u = res->order[i];
        if (res->levels[u] >= 0) {
            if (lvl_count > 0) printf(",");
            print_json_string(g->vertices[u].name);
            printf(":%d", res->levels[u]);
            lvl_count++;
        }
    }
    printf("},\"steps\":[");
    for (int i = 0; i < res->step_count; i++) {
        if (i > 0) printf(",");
        DFSStep* step = &res->steps[i];
        printf("{\"visit\":");
        print_json_string(g->vertices[step->visit_id].name);
        printf(",\"from\":");
        if (step->from_id >= 0) {
            print_json_string(g->vertices[step->from_id].name);
        } else {
            printf("null");
        }
        printf(",\"stack\":[");
        for (int k = 0; k < step->stack_size; k++) {
            if (k > 0) printf(",");
            print_json_string(g->vertices[step->stack_snapshot[k]].name);
        }
        printf("]}");
    }
    printf("]}\n");
    dfs_result_free(res);
}

static void cmd_connect(const Graph* g, const char* name_x, const char* name_y) {
    int u = graph_find_vertex(g, name_x);
    int v = graph_find_vertex(g, name_y);
    if (u < 0 || v < 0) {
        print_error_and_exit("Device not found");
    }

    bool b_conn = bfs_connected(g, u, v);
    bool d_conn = dfs_connected(g, u, v);
    UnionFind* uf = uf_from_graph(g);
    bool u_conn = uf_connected(uf, u, v);
    uf_free(uf);

    bool agree = (b_conn == d_conn && d_conn == u_conn);
    printf("{\"ok\":true,\"bfs\":%s,\"dfs\":%s,\"unionfind\":%s,\"agree\":%s}\n",
           b_conn ? "true" : "false",
           d_conn ? "true" : "false",
           u_conn ? "true" : "false",
           agree ? "true" : "false");
}

static void cmd_path(const Graph* g, const char* src_name, const char* dst_name) {
    int src = graph_find_vertex(g, src_name);
    int dst = graph_find_vertex(g, dst_name);
    if (src < 0 || dst < 0) {
        print_error_and_exit("Device not found");
    }

    DijkstraResult* res = dijkstra_shortest_path(g, src, dst);
    if (!res) {
        print_error_and_exit("Shortest path calculation failed");
    }

    printf("{\"ok\":true,\"reachable\":%s,\"path\":[", res->reachable ? "true" : "false");
    for (int i = 0; i < res->path_len; i++) {
        if (i > 0) printf(",");
        print_json_string(g->vertices[res->path[i]].name);
    }
    printf("],\"cost\":%d,\"edges\":[", res->reachable ? res->total_cost : -1);
    for (int i = 0; i < res->edge_count; i++) {
        if (i > 0) printf(",");
        printf("{\"a\":");
        print_json_string(g->vertices[res->edges[i].u].name);
        printf(",\"b\":");
        print_json_string(g->vertices[res->edges[i].v].name);
        printf(",\"cost\":%d}", res->edges[i].cost);
    }
    printf("],\"steps\":[");
    for (int i = 0; i < res->step_count; i++) {
        if (i > 0) printf(",");
        DijkstraStep* step = &res->steps[i];
        printf("{\"settled\":");
        print_json_string(g->vertices[step->settled_id].name);
        printf(",\"dist\":{");
        int d_count = 0;
        for (int v = 0; v < g->num_vertices; v++) {
            if (step->dist_snapshot[v] < INF_DIST) {
                if (d_count > 0) printf(",");
                print_json_string(g->vertices[v].name);
                printf(":%d", step->dist_snapshot[v]);
                d_count++;
            }
        }
        printf("}}");
    }
    printf("]}\n");
    dijkstra_result_free(res);
}

static void cmd_faults(const Graph* g) {
    FaultResult* res = faults_detect(g);
    if (!res) {
        print_error_and_exit("Fault detection failed");
    }

    printf("{\"ok\":true,\"bridges\":[");
    for (int i = 0; i < res->bridge_count; i++) {
        if (i > 0) printf(",");
        printf("{\"a\":");
        print_json_string(g->vertices[res->bridges[i].u].name);
        printf(",\"b\":");
        print_json_string(g->vertices[res->bridges[i].v].name);
        printf("}");
    }
    printf("],\"articulation\":[");
    for (int i = 0; i < res->articulation_count; i++) {
        if (i > 0) printf(",");
        print_json_string(g->vertices[res->articulation_points[i]].name);
    }
    printf("]}\n");
    fault_result_free(res);
}

static void print_sim_result(SimResult* sres, const Graph* g) {
    printf("{\"ok\":true,\"componentsBefore\":%d,\"componentsAfter\":%d,\"disconnected\":[",
           sres->components_before, sres->components_after);
    if (sres->disconnected_groups) {
        for (int k = 0; k < sres->disconnected_groups->count; k++) {
            if (k > 0) printf(",");
            printf("[");
            for (int j = 0; j < sres->disconnected_groups->groups[k].size; j++) {
                if (j > 0) printf(",");
                int vid = sres->disconnected_groups->groups[k].vertex_ids[j];
                print_json_string(g->vertices[vid].name);
            }
            printf("]");
        }
    }
    printf("]}\n");
}

static void cmd_sim_link(Graph* g, const char* name_a, const char* name_b) {
    int u = graph_find_vertex(g, name_a);
    int v = graph_find_vertex(g, name_b);
    if (u < 0 || v < 0) {
        print_error_and_exit("Device not found for link simulation");
    }
    if (!graph_has_edge(g, u, v)) {
        print_error_and_exit("Link does not exist");
    }

    SimResult* sres = faults_simulate_link(g, name_a, name_b);
    if (!sres) {
        print_error_and_exit("Link simulation failed");
    }
    print_sim_result(sres, g);
    sim_result_free(sres);
}

static void cmd_sim_device(Graph* g, const char* name) {
    int u = graph_find_vertex(g, name);
    if (u < 0) {
        print_error_and_exit("Device not found for simulation");
    }

    SimResult* sres = faults_simulate_device(g, name);
    if (!sres) {
        print_error_and_exit("Device simulation failed");
    }
    print_sim_result(sres, g);
    sim_result_free(sres);
}

int api_main(int argc, char* argv[]) {
    // argv[0] = program, argv[1] = "--api", argv[2] = command, argv[3] = network_file
    if (argc < 4) {
        print_error_and_exit("Usage: analyzer --api <command> <network_file> [args...]");
    }

    const char* command = argv[2];
    const char* network_file = argv[3];

    Graph* g = graph_load_from_file(network_file);
    if (!g) {
        print_error_and_exit("Failed to open or parse network file");
    }

    if (strcmp(command, "summary") == 0) {
        cmd_summary(g);
    } else if (strcmp(command, "bfs") == 0) {
        if (argc < 5) {
            graph_free(g);
            print_error_and_exit("Missing start device for bfs");
        }
        cmd_bfs(g, argv[4]);
    } else if (strcmp(command, "dfs") == 0) {
        if (argc < 5) {
            graph_free(g);
            print_error_and_exit("Missing start device for dfs");
        }
        cmd_dfs(g, argv[4]);
    } else if (strcmp(command, "connect") == 0) {
        if (argc < 6) {
            graph_free(g);
            print_error_and_exit("Missing arguments for connect: need <x> <y>");
        }
        cmd_connect(g, argv[4], argv[5]);
    } else if (strcmp(command, "path") == 0) {
        if (argc < 6) {
            graph_free(g);
            print_error_and_exit("Missing arguments for path: need <src> <dst>");
        }
        cmd_path(g, argv[4], argv[5]);
    } else if (strcmp(command, "faults") == 0) {
        cmd_faults(g);
    } else if (strcmp(command, "simulate-link") == 0) {
        if (argc < 6) {
            graph_free(g);
            print_error_and_exit("Missing arguments for simulate-link: need <a> <b>");
        }
        cmd_sim_link(g, argv[4], argv[5]);
    } else if (strcmp(command, "simulate-device") == 0) {
        if (argc < 5) {
            graph_free(g);
            print_error_and_exit("Missing argument for simulate-device: need <name>");
        }
        cmd_sim_device(g, argv[4]);
    } else {
        graph_free(g);
        print_error_and_exit("Unknown API command");
    }

    graph_free(g);
    return 0;
}
