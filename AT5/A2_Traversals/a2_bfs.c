#include <stdio.h>
#define V 6

int g[V][V] = {
 {0, 1, 1, 0, 0, 0},
 {1, 0, 0, 1, 1, 0},
 {1, 0, 0, 0, 1, 0},
 {0, 1, 0, 0, 0, 1},
 {0, 1, 1, 0, 0, 1},
 {0, 0, 0, 1, 1, 0},
};

int main(void)
{
    int seen[V] = {0};
    int queue[V];
    int head = 0, tail = 0;
    int visit_order[V];
    int visit_count = 0;

    printf("BFS Traversal starting from vertex 0:\n");
    printf("Initial graph has 6 vertices (0 to 5).\n\n");

    seen[0] = 1;
    queue[tail++] = 0;
    printf("Initial queue: [ %d ]\n\n", queue[0]);

    int step = 1;
    while (head < tail) {
        int u = queue[head++];
        visit_order[visit_count++] = u;

        printf("Step %d: Dequeued vertex %d\n", step++, u);
        printf("  Queue immediately after dequeue: [ ");
        for (int i = head; i < tail; i++) {
            printf("%d ", queue[i]);
        }
        printf("]\n");

        /* Explore all neighbours in increasing index order */
        for (int v = 0; v < V; v++) {
            if (g[u][v] && !seen[v]) {
                seen[v] = 1;
                queue[tail++] = v;
                printf("  -> Discovered neighbor %d (enqueued)\n", v);
            }
        }

        printf("  Queue after enqueuing neighbors: [ ");
        for (int i = head; i < tail; i++) {
            printf("%d ", queue[i]);
        }
        printf("]\n\n");
    }

    printf("BFS Visit Order: ");
    for (int i = 0; i < visit_count; i++) {
        printf("%d ", visit_order[i]);
    }
    printf("\n");

    return 0;
}
