#include <stdio.h>

#define V 7

const char *name[V] = {
    "Airport", "Guindy", "Saidapet", "Central", "Egmore", "Vadapalani", "Koyambedu"
};
int start[V + 1] = {0, 1, 4, 6, 8, 10, 12, 14};
int adj[14] = {1, 0, 2, 5, 1, 3, 2, 4, 3, 6, 1, 6, 4, 5};

static void print_queue(const int *q, int head, int tail) {
    printf("[");
    for (int i = head; i < tail; i++) {
        printf(" %s(%d)", name[q[i]], q[i]);
    }
    printf(" ]");
}

int main(void)
{
    int dist[V], parent[V], queue[V * 2], head = 0, tail = 0;
    for (int v = 0; v < V; v++) dist[v] = -1;
    dist[0] = 0;
    parent[0] = -1;
    queue[tail++] = 0;

    printf("========================================================================================\n");
    printf("           CHENNAI METRO BFS STEP-BY-STEP QUEUE TRACE (CSR ADJACENCY)                  \n");
    printf("========================================================================================\n");
    printf("%-6s | %-16s | %-12s | %-40s\n", "Step", "Removed Vertex", "Hop Distance", "Queue Contents After Dequeue");
    printf("----------------------------------------------------------------------------------------\n");

    int step = 1;
    while (head < tail) {
        int u = queue[head++];
        printf("%-6d | %-16s | %-12d | ", step++, name[u], dist[u]);
        print_queue(queue, head, tail);
        printf("\n");

        for (int idx = start[u]; idx < start[u + 1]; idx++) {
            int w = adj[idx];
            if (dist[w] == -1) {
                dist[w] = dist[u] + 1;
                parent[w] = u;
                queue[tail++] = w;
                printf("       -> Enqueued Neighbor : %-16s | Queue: ", name[w]);
                print_queue(queue, head, tail);
                printf("\n");
            }
        }
    }

    printf("========================================================================================\n");
    printf("Shortest Path from Airport (0) to Koyambedu (6):\n");
    printf("Total Stops: %d stops\nPath:", dist[6]);
    for (int v = 6; v != -1; v = parent[v]) printf(" %s", name[v]);
    printf("\n========================================================================================\n");

    return 0;
}
