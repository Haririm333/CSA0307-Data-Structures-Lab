#include "cli.h"
#include "graph.h"
#include "bfs.h"
#include "dfs.h"
#include "unionfind.h"
#include "dijkstra.h"
#include "faults.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_menu(void) {
    printf("\n============================================\n");
    printf("   NETWORK CONNECTIVITY ANALYZER (CLI)     \n");
    printf("============================================\n");
    printf(" 1. Load network from file\n");
    printf(" 2. Display network summary\n");
    printf(" 3. Run BFS traversal\n");
    printf(" 4. Run DFS traversal\n");
    printf(" 5. Check connectivity (BFS / DFS / UF)\n");
    printf(" 6. Shortest path (Dijkstra)\n");
    printf(" 7. Fault detection (Bridges & Cut Vertices)\n");
    printf(" 8. Simulate link failure\n");
    printf(" 9. Simulate device failure\n");
    printf(" 10. Save network to file\n");
    printf(" 0. Exit\n");
    printf("--------------------------------------------\n");
    printf("Enter choice: ");
}

int cli_main(int argc, char* argv[]) {
    Graph* g = NULL;
    if (argc > 1) {
        g = graph_load_from_file(argv[1]);
        if (g) {
            printf("Loaded network from '%s' (%d devices)\n", argv[1], g->num_vertices);
        } else {
            printf("Could not open '%s', starting empty.\n", argv[1]);
        }
    }

    char buf[256];
    while (1) {
        print_menu();
        if (!fgets(buf, sizeof(buf), stdin)) break;
        int choice = atoi(buf);
        if (choice == 0) {
            printf("Exiting Network Analyzer. Goodbye!\n");
            break;
        }

        switch (choice) {
            case 1: {
                printf("Enter filename: ");
                if (fgets(buf, sizeof(buf), stdin)) {
                    buf[strcspn(buf, "\r\n")] = '\0';
                    Graph* ng = graph_load_from_file(buf);
                    if (ng) {
                        if (g) graph_free(g);
                        g = ng;
                        printf("Successfully loaded %d devices.\n", g->num_vertices);
                    } else {
                        printf("Error: Failed to load file '%s'.\n", buf);
                    }
                }
                break;
            }
            case 2: {
                if (!g || g->num_vertices == 0) {
                    printf("No network loaded.\n");
                    break;
                }
                printf("\n--- Network Summary ---\n");
                printf("Devices (%d):\n", g->num_vertices);
                for (int i = 0; i < g->num_vertices; i++) {
                    printf("  [%d] %s (degree %d)\n", g->vertices[i].id, g->vertices[i].name, g->vertices[i].degree);
                }
                ComponentList* comps = uf_get_components(g);
                if (comps) {
                    printf("\nConnected Components (%d):\n", comps->count);
                    for (int k = 0; k < comps->count; k++) {
                        printf("  Component %d: ", k + 1);
                        for (int j = 0; j < comps->groups[k].size; j++) {
                            int v = comps->groups[k].vertex_ids[j];
                            printf("%s ", g->vertices[v].name);
                        }
                        printf("\n");
                    }
                    component_list_free(comps);
                }
                break;
            }
            case 3: {
                if (!g || g->num_vertices == 0) { printf("No network loaded.\n"); break; }
                printf("Enter start device name: ");
                if (fgets(buf, sizeof(buf), stdin)) {
                    buf[strcspn(buf, "\r\n")] = '\0';
                    int s = graph_find_vertex(g, buf);
                    if (s < 0) { printf("Device '%s' not found.\n", buf); break; }
                    BFSResult* res = bfs_traverse(g, s);
                    if (res) {
                        printf("BFS Order: ");
                        for (int i = 0; i < res->order_count; i++) {
                            printf("%s ", g->vertices[res->order[i]].name);
                        }
                        printf("\nLevels:\n");
                        for (int i = 0; i < res->order_count; i++) {
                            int u = res->order[i];
                            printf("  %s: %d\n", g->vertices[u].name, res->levels[u]);
                        }
                        bfs_result_free(res);
                    }
                }
                break;
            }
            case 4: {
                if (!g || g->num_vertices == 0) { printf("No network loaded.\n"); break; }
                printf("Enter start device name: ");
                if (fgets(buf, sizeof(buf), stdin)) {
                    buf[strcspn(buf, "\r\n")] = '\0';
                    int s = graph_find_vertex(g, buf);
                    if (s < 0) { printf("Device '%s' not found.\n", buf); break; }
                    DFSResult* res = dfs_traverse(g, s);
                    if (res) {
                        printf("DFS Order: ");
                        for (int i = 0; i < res->order_count; i++) {
                            printf("%s ", g->vertices[res->order[i]].name);
                        }
                        printf("\nLevels:\n");
                        for (int i = 0; i < res->order_count; i++) {
                            int u = res->order[i];
                            printf("  %s: %d\n", g->vertices[u].name, res->levels[u]);
                        }
                        dfs_result_free(res);
                    }
                }
                break;
            }
            case 5: {
                if (!g || g->num_vertices == 0) { printf("No network loaded.\n"); break; }
                char dev_a[64], dev_b[64];
                printf("Enter device 1: ");
                if (!fgets(dev_a, sizeof(dev_a), stdin)) break;
                dev_a[strcspn(dev_a, "\r\n")] = '\0';
                printf("Enter device 2: ");
                if (!fgets(dev_b, sizeof(dev_b), stdin)) break;
                dev_b[strcspn(dev_b, "\r\n")] = '\0';

                int u = graph_find_vertex(g, dev_a);
                int v = graph_find_vertex(g, dev_b);
                if (u < 0 || v < 0) { printf("One or both devices not found.\n"); break; }

                bool b_conn = bfs_connected(g, u, v);
                bool d_conn = dfs_connected(g, u, v);
                UnionFind* uf = uf_from_graph(g);
                bool u_conn = uf_connected(uf, u, v);
                uf_free(uf);

                printf("Results:\n");
                printf("  BFS:        %s\n", b_conn ? "CONNECTED" : "DISCONNECTED");
                printf("  DFS:        %s\n", d_conn ? "CONNECTED" : "DISCONNECTED");
                printf("  Union-Find: %s\n", u_conn ? "CONNECTED" : "DISCONNECTED");
                printf("  All agree:  %s\n", (b_conn == d_conn && d_conn == u_conn) ? "YES" : "NO");
                break;
            }
            case 6: {
                if (!g || g->num_vertices == 0) { printf("No network loaded.\n"); break; }
                char dev_a[64], dev_b[64];
                printf("Enter source device: ");
                if (!fgets(dev_a, sizeof(dev_a), stdin)) break;
                dev_a[strcspn(dev_a, "\r\n")] = '\0';
                printf("Enter destination device: ");
                if (!fgets(dev_b, sizeof(dev_b), stdin)) break;
                dev_b[strcspn(dev_b, "\r\n")] = '\0';

                int src = graph_find_vertex(g, dev_a);
                int dst = graph_find_vertex(g, dev_b);
                if (src < 0 || dst < 0) { printf("One or both devices not found.\n"); break; }

                DijkstraResult* dres = dijkstra_shortest_path(g, src, dst);
                if (dres) {
                    if (dres->reachable) {
                        printf("Path: ");
                        for (int i = 0; i < dres->path_len; i++) {
                            printf("%s%s", g->vertices[dres->path[i]].name, (i < dres->path_len - 1) ? " -> " : "");
                        }
                        printf("\nTotal Cost: %d\n", dres->total_cost);
                    } else {
                        printf("Destination %s is UNREACHABLE from %s.\n", dev_b, dev_a);
                    }
                    dijkstra_result_free(dres);
                }
                break;
            }
            case 7: {
                if (!g || g->num_vertices == 0) { printf("No network loaded.\n"); break; }
                FaultResult* fres = faults_detect(g);
                if (fres) {
                    printf("\n--- Fault Detection Results ---\n");
                    printf("Bridges (%d):\n", fres->bridge_count);
                    for (int i = 0; i < fres->bridge_count; i++) {
                        printf("  %s <---> %s\n", g->vertices[fres->bridges[i].u].name, g->vertices[fres->bridges[i].v].name);
                    }
                    printf("Articulation Points (%d):\n", fres->articulation_count);
                    for (int i = 0; i < fres->articulation_count; i++) {
                        printf("  %s\n", g->vertices[fres->articulation_points[i]].name);
                    }
                    fault_result_free(fres);
                }
                break;
            }
            case 8: {
                if (!g || g->num_vertices == 0) { printf("No network loaded.\n"); break; }
                char dev_a[64], dev_b[64];
                printf("Enter node 1 of link: ");
                if (!fgets(dev_a, sizeof(dev_a), stdin)) break;
                dev_a[strcspn(dev_a, "\r\n")] = '\0';
                printf("Enter node 2 of link: ");
                if (!fgets(dev_b, sizeof(dev_b), stdin)) break;
                dev_b[strcspn(dev_b, "\r\n")] = '\0';

                SimResult* sres = faults_simulate_link(g, dev_a, dev_b);
                if (sres) {
                    printf("Link Failure Simulation (%s - %s):\n", dev_a, dev_b);
                    printf("  Components before: %d\n", sres->components_before);
                    printf("  Components after:  %d\n", sres->components_after);
                    sim_result_free(sres);
                } else {
                    printf("Failed to simulate link.\n");
                }
                break;
            }
            case 9: {
                if (!g || g->num_vertices == 0) { printf("No network loaded.\n"); break; }
                printf("Enter device to fail: ");
                if (fgets(buf, sizeof(buf), stdin)) {
                    buf[strcspn(buf, "\r\n")] = '\0';
                    SimResult* sres = faults_simulate_device(g, buf);
                    if (sres) {
                        printf("Device Failure Simulation (%s):\n", buf);
                        printf("  Components before: %d\n", sres->components_before);
                        printf("  Components after:  %d\n", sres->components_after);
                        sim_result_free(sres);
                    } else {
                        printf("Failed to simulate device.\n");
                    }
                }
                break;
            }
            case 10: {
                if (!g) { printf("No network loaded.\n"); break; }
                printf("Enter destination filename: ");
                if (fgets(buf, sizeof(buf), stdin)) {
                    buf[strcspn(buf, "\r\n")] = '\0';
                    if (graph_save_to_file(g, buf)) {
                        printf("Network successfully saved to '%s'.\n", buf);
                    } else {
                        printf("Failed to save to '%s'.\n", buf);
                    }
                }
                break;
            }
            default:
                printf("Invalid option.\n");
                break;
        }
    }

    if (g) graph_free(g);
    return 0;
}
