#include <stdio.h>
#include <string.h>

#define N 7

/* Hard-coded sensible 7-vertex Chennai delivery hubs graph */
const char *hub_names[N] = {
    "Central",    /* 0 */
    "Egmore",     /* 1 */
    "T.Nagar",    /* 2 */
    "Guindy",     /* 3 */
    "Adyar",      /* 4 */
    "Velachery",  /* 5 */
    "Tambaram"    /* 6 */
};

/* Undirected adjacency matrix */
int adj[N][N] = {
    /* 0: Central   */ {0, 1, 1, 0, 0, 0, 0},
    /* 1: Egmore    */ {1, 0, 0, 1, 0, 0, 0},
    /* 2: T.Nagar   */ {1, 0, 0, 1, 1, 0, 0},
    /* 3: Guindy    */ {0, 1, 1, 0, 0, 1, 1},
    /* 4: Adyar     */ {0, 0, 1, 0, 0, 1, 0},
    /* 5: Velachery */ {0, 0, 0, 1, 1, 0, 1},
    /* 6: Tambaram  */ {0, 0, 0, 1, 0, 1, 0}
};

static void print_queue(const int *q, int head, int tail) {
    printf("[");
    for (int i = head; i < tail; i++) {
        printf(" %s(%d)", hub_names[q[i]], q[i]);
    }
    printf(" ]");
}

static void print_stack(const int *stk, int top) {
    printf("[");
    for (int i = 0; i <= top; i++) {
        printf(" %s(%d)", hub_names[stk[i]], stk[i]);
    }
    printf(" ]");
}

static void trace_bfs(void) {
    int seen[N] = {0};
    int q[N * 2];
    int head = 0, tail = 0;

    printf("========================================================================================\n");
    printf("                          BFS TRACE ON 7-VERTEX CHENNAI NETWORK                         \n");
    printf("========================================================================================\n");
    printf("%-6s | %-12s | %-16s | %-42s\n", "Step", "Action", "Node Processed", "Queue Contents (Head -> Tail)");
    printf("----------------------------------------------------------------------------------------\n");

    seen[0] = 1;
    q[tail++] = 0;
    int step = 1;
    printf("%-6d | %-12s | %-16s | ", step++, "Enqueue(Start)", "Central(0)");
    print_queue(q, head, tail);
    printf("\n");

    while (head < tail) {
        int u = q[head++];
        char act[32];
        snprintf(act, sizeof(act), "Pop %s(%d)", hub_names[u], u);
        printf("%-6d | %-12s | %-16s | ", step++, "Dequeue", act);
        print_queue(q, head, tail);
        printf("\n");

        for (int v = 0; v < N; v++) {
            if (adj[u][v] && !seen[v]) {
                seen[v] = 1;
                q[tail++] = v;
                char disc[32];
                snprintf(disc, sizeof(disc), "Enq %s(%d)", hub_names[v], v);
                printf("%-6d | %-12s | %-16s | ", step++, "Enqueue", disc);
                print_queue(q, head, tail);
                printf("\n");
            }
        }
    }
    printf("========================================================================================\n\n");
}

static void trace_dfs(void) {
    int seen[N] = {0};
    int stk[N * 4];
    int top = -1;

    printf("========================================================================================\n");
    printf("                          DFS TRACE ON 7-VERTEX CHENNAI NETWORK                         \n");
    printf("========================================================================================\n");
    printf("%-6s | %-12s | %-16s | %-42s\n", "Step", "Action", "Node Processed", "Stack Contents (Bottom -> Top)");
    printf("----------------------------------------------------------------------------------------\n");

    /* Iterative DFS trace matching standard left-to-right neighbor traversal */
    stk[++top] = 0;
    int step = 1;
    printf("%-6d | %-12s | %-16s | ", step++, "Push(Start)", "Central(0)");
    print_stack(stk, top);
    printf("\n");

    while (top >= 0) {
        int u = stk[top--];
        char act[32];
        snprintf(act, sizeof(act), "%s(%d)", hub_names[u], u);

        if (!seen[u]) {
            seen[u] = 1;
            printf("%-6d | %-12s | %-16s | ", step++, "Pop & Visit", act);
            print_stack(stk, top);
            printf("\n");

            /* Push in reverse order so smaller indices are popped first */
            for (int v = N - 1; v >= 0; v--) {
                if (adj[u][v] && !seen[v]) {
                    stk[++top] = v;
                    char push_act[32];
                    snprintf(push_act, sizeof(push_act), "Push %s(%d)", hub_names[v], v);
                    printf("%-6d | %-12s | %-16s | ", step++, "Push Neighbor", push_act);
                    print_stack(stk, top);
                    printf("\n");
                }
            }
        } else {
            printf("%-6d | %-12s | %-16s | ", step++, "Pop(Already Seen)", act);
            print_stack(stk, top);
            printf("\n");
        }
    }
    printf("========================================================================================\n");
}

int main(void) {
    trace_bfs();
    trace_dfs();
    return 0;
}
