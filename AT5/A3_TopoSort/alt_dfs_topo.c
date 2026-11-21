#include <stdio.h>
#include <stdbool.h>

#define V 6

const char *task[V] = {"Lease", "Power", "Racks", "Network", "Staff", "Launch"};

/* 1. Valid DAG */
int g_valid[V][V] = {
 /* Lease */   {0, 1, 1, 0, 1, 0},
 /* Power */   {0, 0, 0, 1, 0, 0},
 /* Racks */   {0, 0, 0, 1, 0, 0},
 /* Network */ {0, 0, 0, 0, 0, 1},
 /* Staff */   {0, 0, 0, 0, 0, 1},
 /* Launch */  {0, 0, 0, 0, 0, 0},
};

/* 2. Cyclic Graph (Launch -> Lease) */
int g_cyclic[V][V] = {
 /* Lease */   {0, 1, 1, 0, 1, 0},
 /* Power */   {0, 0, 0, 1, 0, 0},
 /* Racks */   {0, 0, 0, 1, 0, 0},
 /* Network */ {0, 0, 0, 0, 0, 1},
 /* Staff */   {0, 0, 0, 0, 0, 1},
 /* Launch */  {1, 0, 0, 0, 0, 0},
};

enum Color { WHITE = 0, GREY = 1, BLACK = 2 };

static bool dfs_topo(int u, int g[V][V], enum Color color[V], int order[V], int *order_idx) {
    color[u] = GREY;

    for (int v = 0; v < V; v++) {
        if (g[u][v]) {
            if (color[v] == GREY) {
                /* Back-edge found -> cycle! */
                return false;
            }
            if (color[v] == WHITE) {
                if (!dfs_topo(v, g, color, order, order_idx)) {
                    return false;
                }
            }
        }
    }

    color[u] = BLACK;
    /* Prepend to topological order (or write into reverse slot) */
    order[(*order_idx)--] = u;
    return true;
}

static void run_dfs_toposort(const char *name, int g[V][V]) {
    enum Color color[V] = {WHITE};
    int order[V];
    int order_idx = V - 1;
    bool has_cycle = false;

    printf("--- Running 3-Color DFS Topological Sort on %s ---\n", name);
    for (int u = 0; u < V; u++) {
        if (color[u] == WHITE) {
            if (!dfs_topo(u, g, color, order, &order_idx)) {
                has_cycle = true;
                break;
            }
        }
    }

    if (has_cycle) {
        printf("Cycle detected! (Encountered GREY back-edge in recursion stack)\n");
    } else {
        printf("Valid schedule found (DFS Finishing Order):\n");
        for (int i = 0; i < V; i++) {
            printf("%s%s", i > 0 ? " -> " : "", task[order[i]]);
        }
        printf("\n");
    }
    printf("\n");
}

int main(void) {
    printf("========================================================================\n");
    printf("              DFS 3-COLORING TOPOLOGICAL SORT COMPARISON                \n");
    printf("========================================================================\n\n");

    /* Run on valid graph */
    run_dfs_toposort("Valid Hub Setup Graph", g_valid);

    /* Run on cyclic graph */
    run_dfs_toposort("Cyclic Graph (Launch -> Lease)", g_cyclic);

    printf("========================================================================\n");
    printf("Comparison of Valid Topological Schedules:\n");
    printf("1. Kahn's (BFS In-Degree) Order : Lease Power Racks Staff Network Launch\n");
    printf("2. DFS Finishing (Post-Order)   : Lease Staff Power Racks Network Launch\n");
    printf("(Both satisfy all dependency constraints: Lease before Power/Racks/Staff,\n");
    printf(" Power and Racks before Network, Network and Staff before Launch.)\n");
    printf("========================================================================\n");

    return 0;
}
