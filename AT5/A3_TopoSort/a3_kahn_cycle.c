#include <stdio.h>
#include <stdbool.h>

#define V 6
/* Delivery-hub setup tasks; edge u -> v means u must finish before v. */
const char *task[V] = {"Lease", "Power", "Racks", "Network", "Staff", "Launch"};

/* Cyclic graph: Launch -> Lease creates a cycle (Launch cannot happen before Lease, Lease cannot happen before Launch) */
int g[V][V] = {
 /* Lease */   {0, 1, 1, 0, 1, 0},
 /* Power */   {0, 0, 0, 1, 0, 0},
 /* Racks */   {0, 0, 0, 1, 0, 0},
 /* Network */ {0, 0, 0, 0, 0, 1},
 /* Staff */   {0, 0, 0, 0, 0, 1},
 /* Launch */  {1, 0, 0, 0, 0, 0}, /* CYCLE EDGE: Launch -> Lease */
};

int main(void)
{
    int indeg[V] = {0}, queue[V], head = 0, tail = 0, done = 0;
    bool outputted[V] = {false};

    for (int u = 0; u < V; u++)
        for (int v = 0; v < V; v++)
            indeg[v] += g[u][v];

    for (int v = 0; v < V; v++)
        if (indeg[v] == 0)
            queue[tail++] = v;

    printf("Executing Kahn's Algorithm on Hub Setup with Launch -> Lease back-edge:\n");
    printf("Processed tasks: ");
    while (head < tail) {
        int u = queue[head++];
        if (done > 0) printf(" ");
        printf("%s", task[u]);
        outputted[u] = true;
        done++;
        for (int v = 0; v < V; v++) {
            if (g[u][v] && --indeg[v] == 0) {
                queue[tail++] = v;
            }
        }
    }

    if (done == 0) {
        printf("(none)");
    }
    printf("\n%s\n", done == V ? "valid schedule" : "cycle detected");

    if (done < V) {
        printf("\nUnresolved / Stuck Tasks (in-degree remaining > 0):\n");
        for (int v = 0; v < V; v++) {
            if (!outputted[v]) {
                printf(" - Task %d: %s (remaining in-degree = %d)\n", v, task[v], indeg[v]);
            }
        }
    }

    return 0;
}
