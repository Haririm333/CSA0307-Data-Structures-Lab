#include <stdio.h>
#include <stdbool.h>

#define V 6

const char *task[V] = {"Lease", "Power", "Racks", "Network", "Staff", "Launch"};

/* Hub setup with circular dependency: Launch -> Lease */
int g[V][V] = {
 /* Lease */   {0, 1, 1, 0, 1, 0},
 /* Power */   {0, 0, 0, 1, 0, 0},
 /* Racks */   {0, 0, 0, 1, 0, 0},
 /* Network */ {0, 0, 0, 0, 0, 1},
 /* Staff */   {0, 0, 0, 0, 0, 1},
 /* Launch */  {1, 0, 0, 0, 0, 0}, /* Back-edge creating cycle */
};

int main(void)
{
    int indeg[V] = {0}, queue[V], head = 0, tail = 0, done = 0;
    bool scheduled[V] = {false};

    for (int u = 0; u < V; u++)
        for (int v = 0; v < V; v++)
            indeg[v] += g[u][v];

    for (int v = 0; v < V; v++)
        if (indeg[v] == 0)
            queue[tail++] = v;

    printf("========================================================================\n");
    printf("        KAHN'S ALGORITHM UNDER CYCLIC DEPENDENCY GRAPH                  \n");
    printf("========================================================================\n\n");

    printf("Initial In-Degrees:\n");
    for (int v = 0; v < V; v++) {
        printf(" - %-10s : in-degree = %d\n", task[v], indeg[v]);
    }
    printf("\nExecution Trace:\n");

    while (head < tail) {
        int u = queue[head++];
        scheduled[u] = true;
        done++;
        printf(">> Scheduled: %s (in-degree became 0)\n", task[u]);
        for (int v = 0; v < V; v++) {
            if (g[u][v] && --indeg[v] == 0) {
                queue[tail++] = v;
            }
        }
    }

    printf("\nStatus: %s\n", done == V ? "VALID SCHEDULE" : "CYCLE DETECTED (DEADLOCK)");
    printf("Total tasks scheduled: %d / %d\n\n", done, V);

    if (done < V) {
        printf("TASKS STUCK IN DEADLOCK CYCLE (In-degree never reached 0):\n");
        for (int v = 0; v < V; v++) {
            if (!scheduled[v]) {
                printf(" - %-10s (Remaining in-degree = %d, awaiting prerequisite completion)\n",
                       task[v], indeg[v]);
            }
        }
        printf("\nDependency Cycle Pathway:\n");
        printf("  Launch -> Lease -> {Power, Racks} -> Network -> Launch\n");
        printf("  Launch -> Lease -> Staff -> Launch\n");
    }

    printf("========================================================================\n");
    return 0;
}
