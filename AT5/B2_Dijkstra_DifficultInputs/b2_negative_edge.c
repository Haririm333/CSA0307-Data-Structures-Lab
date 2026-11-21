#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

#define V 3
#define INF 999999

/* Directed graph with a negative edge:
   0 -> 1 (weight 2)
   0 -> 2 (weight 3)
   2 -> 1 (weight -2)
*/
typedef struct {
    int u, v, w;
} Edge;

static Edge edges[3] = {
    {0, 1, 2},
    {0, 2, 3},
    {2, 1, -2}
};
static int num_edges = 3;

static void run_dijkstra(int dist[V]) {
    int done[V] = {0};
    for (int i = 0; i < V; i++) dist[i] = INF;
    dist[0] = 0;

    for (int step = 0; step < V; step++) {
        int u = -1;
        for (int v = 0; v < V; v++) {
            if (!done[v] && dist[v] != INF && (u == -1 || dist[v] < dist[u])) {
                u = v;
            }
        }
        if (u == -1) break;
        done[u] = 1;

        for (int e = 0; e < num_edges; e++) {
            if (edges[e].u == u) {
                int v = edges[e].v;
                int w = edges[e].w;
                /* Standard Dijkstra relaxation: only relaxes unvisited (done == 0) vertices */
                if (!done[v] && dist[u] + w < dist[v]) {
                    dist[v] = dist[u] + w;
                }
            }
        }
    }
}

static void run_bellman_ford(int dist[V]) {
    for (int i = 0; i < V; i++) dist[i] = INF;
    dist[0] = 0;

    for (int i = 0; i < V - 1; i++) {
        for (int e = 0; e < num_edges; e++) {
            int u = edges[e].u;
            int v = edges[e].v;
            int w = edges[e].w;
            if (dist[u] != INF && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
            }
        }
    }
}

static void run_floyd_warshall(int dist[V]) {
    int d[V][V];
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            d[i][j] = (i == j) ? 0 : INF;
        }
    }
    for (int e = 0; e < num_edges; e++) {
        d[edges[e].u][edges[e].v] = edges[e].w;
    }

    for (int k = 0; k < V; k++) {
        for (int i = 0; i < V; i++) {
            for (int j = 0; j < V; j++) {
                if (d[i][k] != INF && d[k][j] != INF && d[i][k] + d[k][j] < d[i][j]) {
                    d[i][j] = d[i][k] + d[k][j];
                }
            }
        }
    }
    for (int i = 0; i < V; i++) dist[i] = d[0][i];
}

int main(void) {
    printf("========================================================================================\n");
    printf("        FAILURE OF DIJKSTRA ON GRAPHS WITH NEGATIVE EDGES                               \n");
    printf("========================================================================================\n\n");
    printf("Graph Topology:\n");
    printf(" - Edge 0 -> 1 (weight +2)\n");
    printf(" - Edge 0 -> 2 (weight +3)\n");
    printf(" - Edge 2 -> 1 (weight -2)\n\n");

    int dist_dijkstra[V];
    int dist_bellman[V];
    int dist_floyd[V];

    run_dijkstra(dist_dijkstra);
    run_bellman_ford(dist_bellman);
    run_floyd_warshall(dist_floyd);

    printf("%-8s | %-16s | %-16s | %-16s | %-12s\n",
           "Vertex", "Dijkstra dist[v]", "Bellman-Ford", "Floyd-Warshall", "Correctness");
    printf("----------------------------------------------------------------------------------------\n");

    for (int v = 0; v < V; v++) {
        bool correct = (dist_dijkstra[v] == dist_bellman[v]);
        printf("%-8d | %-16d | %-16d | %-16d | %s\n",
               v, dist_dijkstra[v], dist_bellman[v], dist_floyd[v],
               correct ? "MATCH" : "WRONG (FAILURE)");
    }

    printf("========================================================================================\n");
    printf("ANALYSIS OF THE GREEDY CHOICE PROPERTY VIOLATION:\n");
    printf("1. Dijkstra selects vertex 1 at Step 1 because dist[1] = 2 < dist[2] = 3.\n");
    printf("2. Greedy invariant assumes: 'Once marked done, dist[1] can never decrease further'.\n");
    printf("3. With negative edge (2 -> 1 = -2), routing through vertex 2 gives cost 3 + (-2) = 1.\n");
    printf("4. Because vertex 1 is already closed, Dijkstra never re-evaluates it, yielding dist[1] = 2 (WRONG).\n");
    printf("5. Both Bellman-Ford and Floyd-Warshall correctly discover dist[1] = 1.\n");
    printf("========================================================================================\n");

    return 0;
}
