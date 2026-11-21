#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

#define V 5

/* Cost (Rs lakh) of laying fibre cable between villages (0 = not possible). */
int c[V][V] = {
 {0, 2, 0, 6, 0},
 {2, 0, 3, 8, 5},
 {0, 3, 0, 0, 7},
 {6, 8, 0, 0, 9},
 {0, 5, 7, 9, 0},
};

static void compute_dijkstra(int src, int dst, int *cost, int path[], int *path_len) {
    int dist[V], prev[V], done[V] = {0};
    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;
        prev[i] = -1;
    }
    dist[src] = 0;

    for (int step = 0; step < V; step++) {
        int u = -1;
        for (int v = 0; v < V; v++) {
            if (!done[v] && dist[v] != INT_MAX && (u == -1 || dist[v] < dist[u])) {
                u = v;
            }
        }
        if (u == -1) break;
        done[u] = 1;

        for (int v = 0; v < V; v++) {
            if (c[u][v] > 0 && !done[v] && dist[u] + c[u][v] < dist[v]) {
                dist[v] = dist[u] + c[u][v];
                prev[v] = u;
            }
        }
    }

    *cost = dist[dst];
    int cur = dst;
    int len = 0;
    while (cur != -1) {
        path[len++] = cur;
        cur = prev[cur];
    }
    *path_len = len;
}

/* Find path in MST from src to dst using DFS */
static bool dfs_mst(int u, int target, int parent_edge, const int mst_adj[V][V], int path[], int *len) {
    path[(*len)++] = u;
    if (u == target) return true;

    for (int v = 0; v < V; v++) {
        if (v != parent_edge && mst_adj[u][v] > 0) {
            if (dfs_mst(v, target, u, mst_adj, path, len)) {
                return true;
            }
        }
    }
    (*len)--;
    return false;
}

int main(void) {
    printf("========================================================================\n");
    printf("         SHORTEST PATH vs MINIMUM SPANNING TREE PATH COMPARISON         \n");
    printf("========================================================================\n\n");

    /* 1. Compute MST using Prim */
    int key[V], parent[V], in[V] = {0}, total = 0;
    for (int v = 0; v < V; v++) { key[v] = INT_MAX; parent[v] = -1; }
    key[0] = 0;

    for (int k = 0; k < V; k++) {
        int u = -1;
        for (int v = 0; v < V; v++) {
            if (!in[v] && (u == -1 || key[v] < key[u])) u = v;
        }
        if (u == -1) break;
        in[u] = 1;
        total += key[u];
        for (int v = 0; v < V; v++) {
            if (!in[v] && c[u][v] > 0 && c[u][v] < key[v]) {
                key[v] = c[u][v];
                parent[v] = u;
            }
        }
    }

    int mst_adj[V][V] = {0};
    for (int v = 1; v < V; v++) {
        mst_adj[parent[v]][v] = c[parent[v]][v];
        mst_adj[v][parent[v]] = c[parent[v]][v];
    }

    printf("Prim's MST Total Cost: %d lakh\n", total);
    printf("MST Edges: ");
    for (int v = 1; v < V; v++) {
        printf("%d-%d(%d) ", parent[v], v, c[parent[v]][v]);
    }
    printf("\n\n");

    /* 2. Compute Shortest Path 4 -> 2 using Dijkstra */
    int sp_cost = 0;
    int sp_path[V];
    int sp_len = 0;
    compute_dijkstra(4, 2, &sp_cost, sp_path, &sp_len);

    printf("1. Shortest Path (Dijkstra) between Node 4 and Node 2:\n");
    printf("   - Route: ");
    for (int i = sp_len - 1; i >= 0; i--) {
        printf("%d%s", sp_path[i], i > 0 ? " -> " : "");
    }
    printf("\n   - Cost: %d lakh (direct road 4-2 exists with cost 7)\n\n", sp_cost);

    /* 3. Compute Path 4 -> 2 inside the MST */
    int mst_path[V];
    int mst_len = 0;
    dfs_mst(4, 2, -1, mst_adj, mst_path, &mst_len);

    int mst_path_cost = 0;
    for (int i = 0; i < mst_len - 1; i++) {
        mst_path_cost += c[mst_path[i]][mst_path[i + 1]];
    }

    printf("2. Path inside MST between Node 4 and Node 2:\n");
    printf("   - Route: ");
    for (int i = 0; i < mst_len; i++) {
        printf("%d%s", mst_path[i], i < mst_len - 1 ? " -> " : "");
    }
    printf("\n   - Cost: %d lakh (4 -> 1 is 5, 1 -> 2 is 3, total = 8)\n\n", mst_path_cost);

    printf("========================================================================\n");
    printf("ENGINEERING CONCLUSION:\n");
    printf("An MST minimizes the GLOBAL SUM of all edges in the spanning tree (%d lakh),\n", total);
    printf("guaranteeing total network connectivity with minimal infrastructure expenditure.\n");
    printf("However, the MST does NOT guarantee point-to-point shortest paths:\n");
    printf("Direct road 4-2 costs %d lakh, whereas routing through the MST costs %d lakh.\n", sp_cost, mst_path_cost);
    printf("========================================================================\n");

    return 0;
}
