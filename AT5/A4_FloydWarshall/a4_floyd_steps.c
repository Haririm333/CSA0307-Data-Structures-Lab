#include <stdio.h>

#define V 4
#define X 999 /* no direct road */

static void print_matrix(int d[V][V]) {
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            if (d[i][j] >= X) {
                printf(" INF");
            } else {
                printf("%4d", d[i][j]);
            }
        }
        printf("\n");
    }
}

int main(void)
{
    int d[V][V] = {
        {0, 5, X, 10},
        {X, 0, 3, X},
        {X, X, 0, 1},
        {X, X, X, 0},
    };

    printf("========================================================================\n");
    printf("        FLOYD-WARSHALL STEP-BY-STEP TRACE (INTERMEDIATE MATRICES)       \n");
    printf("========================================================================\n");
    printf("Initial Matrix (D^(0) - direct edges only):\n");
    print_matrix(d);
    printf("\n");

    for (int k = 0; k < V; k++) {
        for (int i = 0; i < V; i++) {
            for (int j = 0; j < V; j++) {
                if (d[i][k] != X && d[k][j] != X && d[i][k] + d[k][j] < d[i][j]) {
                    d[i][j] = d[i][k] + d[k][j];
                }
            }
        }
        printf("--- Matrix after allowing intermediate vertex k = %d ---\n", k);
        print_matrix(d);
        if (k == 1) {
            printf(">> Note at k = 1: d[0][2] reduced from INF to %d via path (0 -> 1 -> 2 = 5 + 3)\n", d[0][2]);
        }
        if (k == 2) {
            printf(">> Note at k = 2: d[0][3] reduced from 10 to %d via intermediate vertex 2: d[0][2] + d[2][3] = %d + %d = %d\n",
                   d[0][3], d[0][2], 1, d[0][3]);
        }
        printf("\n");
    }

    printf("========================================================================\n");
    printf("EXPLANATION OF WHY d[0][3] CHANGED FROM 10 TO 9:\n");
    printf("Direct edge (0 -> 3) had weight 10.\n");
    printf("When intermediate vertex k=1 is relaxed, d[0][2] is discovered: 0 -> 1 (5) + 1 -> 2 (3) = 8.\n");
    printf("When intermediate vertex k=2 is relaxed, path 0 -> 2 -> 3 is evaluated: d[0][2] (8) + d[2][3] (1) = 9.\n");
    printf("Since 9 < 10, d[0][3] is updated to 9, corresponding to path 0 -> 1 -> 2 -> 3 (5 + 3 + 1 = 9).\n");
    printf("========================================================================\n");

    return 0;
}
