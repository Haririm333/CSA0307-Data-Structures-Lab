#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../common/timer.h"

/* Simple singly-linked adjacency list for large graphs */
typedef struct AdjNode {
    int v;
    struct AdjNode *next;
} AdjNode;

typedef struct {
    int n;
    AdjNode **head;
} Graph;

static Graph *create_chain_graph(int n) {
    Graph *g = (Graph *)malloc(sizeof(Graph));
    if (!g) return NULL;
    g->n = n;
    g->head = (AdjNode **)calloc((size_t)n, sizeof(AdjNode *));
    if (!g->head) {
        free(g);
        return NULL;
    }

    /* Chain: 0 -> 1 -> 2 -> ... -> n-1 */
    for (int i = 0; i < n - 1; i++) {
        AdjNode *node = (AdjNode *)malloc(sizeof(AdjNode));
        if (!node) break;
        node->v = i + 1;
        node->next = g->head[i];
        g->head[i] = node;
    }
    return g;
}

static void free_graph(Graph *g) {
    if (!g) return;
    for (int i = 0; i < g->n; i++) {
        AdjNode *curr = g->head[i];
        while (curr) {
            AdjNode *tmp = curr->next;
            free(curr);
            curr = tmp;
        }
    }
    free(g->head);
    free(g);
}

/* Recursive DFS (guarded against tail-call elimination to demonstrate stack depth) */
static volatile int g_recurse_depth = 0;
static int g_max_recurse_depth = 0;

__attribute__((noinline))
static void dfs_recursive(const Graph *g, int u, bool *seen, int *visit_count) {
    seen[u] = true;
    (*visit_count)++;
    g_recurse_depth++;
    if (g_recurse_depth > g_max_recurse_depth) {
        g_max_recurse_depth = g_recurse_depth;
    }

    for (AdjNode *curr = g->head[u]; curr != NULL; curr = curr->next) {
        int v = curr->v;
        if (!seen[v]) {
            dfs_recursive(g, v, seen, visit_count);
        }
    }

    g_recurse_depth--;
}

/* Iterative DFS using an explicit heap-allocated stack */
typedef struct {
    int u;
    AdjNode *curr;
} StackFrame;

static void dfs_iterative(const Graph *g, int start, bool *seen, int *visit_count, int *out_max_stack) {
    StackFrame *stack = (StackFrame *)malloc(sizeof(StackFrame) * (size_t)g->n);
    if (!stack) {
        fprintf(stderr, "Allocation failed for explicit stack.\n");
        return;
    }

    int top = 0;
    int max_depth = 0;

    seen[start] = true;
    (*visit_count)++;
    stack[top].u = start;
    stack[top].curr = g->head[start];

    while (top >= 0) {
        if (top + 1 > max_depth) {
            max_depth = top + 1;
        }

        StackFrame *frame = &stack[top];
        if (frame->curr != NULL) {
            int v = frame->curr->v;
            frame->curr = frame->curr->next;
            if (!seen[v]) {
                seen[v] = true;
                (*visit_count)++;
                top++;
                stack[top].u = v;
                stack[top].curr = g->head[v];
            }
        } else {
            top--;
        }
    }

    *out_max_stack = max_depth;
    free(stack);
}

int main(int argc, char **argv) {
    bool run_crash = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--crash") == 0) {
            run_crash = true;
        }
    }

    printf("========================================================================\n");
    printf("        RECURSIVE vs ITERATIVE DFS (STACK OVERFLOW RISK STUDY)          \n");
    printf("========================================================================\n");

    /* TEST 1: Chain of 50,000 junctions */
    int n50k = 50000;
    printf("\n--- Test 1: Linear Chain of %d Junctions ---\n", n50k);
    Graph *g50k = create_chain_graph(n50k);
    if (!g50k) {
        fprintf(stderr, "Failed to create 50,000 junction graph.\n");
        return 1;
    }

    bool *seen = (bool *)calloc((size_t)n50k, sizeof(bool));
    int count = 0;
    int max_stack = 0;

    /* Iterative DFS */
    double t0 = get_time_sec();
    dfs_iterative(g50k, 0, seen, &count, &max_stack);
    double t1 = get_time_sec();
    printf("1. Explicit Iterative DFS: SUCCESS\n");
    printf("   - Visited: %d / %d junctions\n", count, n50k);
    printf("   - Max explicit stack depth: %d\n", max_stack);
    printf("   - Time: %.6f seconds\n", t1 - t0);

    /* Recursive DFS on 50k */
    memset(seen, 0, sizeof(bool) * (size_t)n50k);
    count = 0;
    g_recurse_depth = 0;
    g_max_recurse_depth = 0;
    t0 = get_time_sec();
    dfs_recursive(g50k, 0, seen, &count);
    t1 = get_time_sec();
    printf("2. Call-Stack Recursive DFS: SUCCESS\n");
    printf("   - Visited: %d / %d junctions\n", count, n50k);
    printf("   - Max recursion depth: %d\n", g_max_recurse_depth);
    printf("   - Time: %.6f seconds\n", t1 - t0);

    free(seen);
    free_graph(g50k);

    /* TEST 2: Chain of 1,000,000 junctions */
    int n1m = 1000000;
    printf("\n--- Test 2: Linear Chain of %d Junctions (Scale Stress Test) ---\n", n1m);
    Graph *g1m = create_chain_graph(n1m);
    if (!g1m) {
        fprintf(stderr, "Failed to create 1,000,000 junction graph.\n");
        return 1;
    }

    seen = (bool *)calloc((size_t)n1m, sizeof(bool));
    count = 0;
    max_stack = 0;

    t0 = get_time_sec();
    dfs_iterative(g1m, 0, seen, &count, &max_stack);
    t1 = get_time_sec();
    printf("1. Explicit Iterative DFS: SUCCESS\n");
    printf("   - Visited: %d / %d junctions\n", count, n1m);
    printf("   - Max explicit stack depth: %d\n", max_stack);
    printf("   - Time: %.6f seconds\n", t1 - t0);

    if (run_crash) {
        printf("2. Call-Stack Recursive DFS (--crash requested): Executing recursion to 1,000,000 depth...\n");
        fflush(stdout);
        memset(seen, 0, sizeof(bool) * (size_t)n1m);
        count = 0;
        g_recurse_depth = 0;
        g_max_recurse_depth = 0;
        dfs_recursive(g1m, 0, seen, &count);
        printf("   - Visited: %d / %d junctions\n", count, n1m);
        printf("   - Max recursion depth: %d\n", g_max_recurse_depth);
    } else {
        printf("2. Call-Stack Recursive DFS: GUARDED (Omitted by default to prevent segmentation fault / stack overflow)\n");
        printf("   - Theoretical call stack required: 1,000,000 frames * ~48 bytes = ~48 MB.\n");
        printf("   - System stack limits: Linux default 'ulimit -s' is 8192 KB (8 MB); Windows default is 1 MB.\n");
        printf("   - To force crash on your platform, run with: ./alt_dfs_iterative --crash\n");
    }

    free(seen);
    free_graph(g1m);

    printf("========================================================================\n");
    return 0;
}
