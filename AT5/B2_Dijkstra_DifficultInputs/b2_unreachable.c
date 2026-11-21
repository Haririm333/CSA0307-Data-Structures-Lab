#include <stdio.h>
#include <limits.h>

#define V 6
#define INF INT_MAX

/* Graph with disconnected component:
   Component 1: {0, 1, 2, 3} (connected urban cluster)
   Component 2: {4, 5} (island / offshore delivery hubs with no bridge to cluster 1)
*/
int w[V][V] = {
    {0, 7, 9, 0, 0, 0},
    {7, 0, 10, 15, 0, 0},
    {9, 10, 0, 11, 0, 0},
    {0, 15, 11, 0, 0, 0},
    {0, 0, 0, 0, 0, 4},
    {0, 0, 0, 0, 4, 0}
};

int main(void) {
    int dist[V], prev[V], done[V] = {0};
    for (int v = 0; v < V; v++) { dist[v] = INF; prev[v] = -1; }
    dist[0] = 0;

    printf("========================================================================\n");
    printf("        DIJKSTRA WITH UNREACHABLE / DISCONNECTED VERTICES               \n");
    printf("========================================================================\n\n");

    for (int round = 0; round < V; round++) {
        int u = -1;
        for (int v = 0; v < V; v++) {
            if (!done[v] && dist[v] != INF && (u == -1 || dist[v] < dist[u])) {
                u = v;
            }
        }
        if (u == -1) {
            printf("[Observation: Priority queue emptied early at round %d because remaining vertices have dist == INF]\n", round);
            break;
        }
        done[u] = 1;

        for (int v = 0; v < V; v++) {
            if (w[u][v] > 0 && !done[v] && dist[u] + w[u][v] < dist[v]) {
                dist[v] = dist[u] + w[u][v];
                prev[v] = u;
            }
        }
    }

    printf("\nComputed Distances from Source Vertex 0:\n");
    for (int v = 0; v < V; v++) {
        if (dist[v] == INF) {
            printf("Vertex %d: unreachable\n", v);
        } else {
            printf("Vertex %d: %d min\n", v, dist[v]);
        }
    }

    printf("\nRoute Queries from Vertex 0:\n");
    for (int target = 0; target < V; target++) {
        if (dist[target] == INF) {
            printf("Target %d: no route to %d\n", target, target);
        } else {
            printf("Target %d (cost %d min):", target, dist[target]);
            for (int v = target; v != -1; v = prev[v]) {
                printf(" %d", v);
            }
            printf("\n");
        }
    }

    printf("========================================================================\n");
    return 0;
}
