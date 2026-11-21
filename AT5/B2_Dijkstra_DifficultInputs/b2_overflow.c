#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

#define V 3
#define INF_INT INT_MAX
#define INF_LL  LLONG_MAX

/* Travel times near INT_MAX / 2:
   edge 0 -> 1 = 1,500,000,000
   edge 1 -> 2 = 1,500,000,000
   Sum along path 0 -> 1 -> 2 = 3,000,000,000 > INT_MAX (2,147,483,647)
*/
static const int W1 = 1500000000;
static const int W2 = 1500000000;

/* 1. BUGGY VERSION: 32-bit signed addition.
   To demonstrate the bug reproducibly without invoking undefined behavior (UB),
   we perform 32-bit two's complement wrapping via unsigned conversion.
*/
static void run_buggy_overflow(void) {
    int dist[V];
    for (int i = 0; i < V; i++) dist[i] = INF_INT;
    dist[0] = 0;

    printf("1. BUGGY DIJKSTRA (32-bit Two's Complement Wrapping):\n");
    /* Relax 0 -> 1 */
    dist[1] = dist[0] + W1;
    printf("   - dist[1] = %d (1,500,000,000)\n", dist[1]);

    /* Relax 1 -> 2: 1,500,000,000 + 1,500,000,000 wraps around */
    unsigned int wrapped = (unsigned int)dist[1] + (unsigned int)W2;
    int buggy_sum = (int)wrapped;
    dist[2] = buggy_sum;

    printf("   - dist[2] = dist[1] + W2 = %d (NEGATIVE OVERFLOW!)\n", dist[2]);
    printf("   - Result: dist[2] evaluated as %d < dist[0] (0), corrupting shortest path logic.\n\n", dist[2]);
}

/* 2. FIX A: Using 64-bit long long for distance accumulations */
static void run_fix_long_long(void) {
    long long dist[V];
    for (int i = 0; i < V; i++) dist[i] = INF_LL;
    dist[0] = 0;

    printf("2. FIX A (64-bit 'long long' Accumulator):\n");
    dist[1] = dist[0] + (long long)W1;
    dist[2] = dist[1] + (long long)W2;

    printf("   - dist[1] = %lld\n", dist[1]);
    printf("   - dist[2] = %lld (Exact true sum: 3,000,000,000, safe from overflow)\n\n", dist[2]);
}

/* 3. FIX B: Overflow check before addition (w <= INF - dist[u]) */
static void run_fix_overflow_guard(void) {
    int dist[V];
    for (int i = 0; i < V; i++) dist[i] = INF_INT;
    dist[0] = 0;

    printf("3. FIX B (Explicit Saturation Guard: w <= INF - dist[u]):\n");
    /* Relax 0 -> 1 */
    if (W1 <= INF_INT - dist[0]) {
        dist[1] = dist[0] + W1;
    } else {
        dist[1] = INF_INT; /* Saturate */
    }

    /* Relax 1 -> 2 */
    if (W2 <= INF_INT - dist[1]) {
        dist[2] = dist[1] + W2;
    } else {
        printf("   - Detected impending overflow: W2 (%d) > INF - dist[1] (%d)\n", W2, INF_INT - dist[1]);
        dist[2] = INF_INT; /* Clamped to infinity rather than wrapping negative */
    }
    printf("   - dist[1] = %d\n", dist[1]);
    printf("   - dist[2] = %d (Safely saturated to INT_MAX)\n\n", dist[2]);
}

/* 4. Guard against unvisited INF + w overflow */
static void run_unvisited_inf_guard(void) {
    int dist_unvisited = INF_INT;
    int weight = 50;

    printf("4. UNVISITED VERTEX INF + W GUARD:\n");
    printf("   - Without guard: INF + 50 wraps around to: %d (corrupts min-heap)\n",
           (int)((unsigned int)dist_unvisited + (unsigned int)weight));

    /* Safe guard: skip relaxation if dist[u] == INF */
    printf("   - With guard 'if (dist[u] != INF)': relaxation is safely skipped for unreached nodes.\n");
}

int main(void) {
    printf("========================================================================\n");
    printf("        INTEGER OVERFLOW VULNERABILITIES IN DIJKSTRA RELAXATION         \n");
    printf("========================================================================\n\n");

    run_buggy_overflow();
    run_fix_long_long();
    run_fix_overflow_guard();
    run_unvisited_inf_guard();

    printf("========================================================================\n");
    return 0;
}
