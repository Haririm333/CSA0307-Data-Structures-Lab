#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <limits.h>
#include <string.h>

#define N 20

typedef struct {
    const char *name;
    double x, y; /* Coordinates in km */
} Hub;

static const Hub hubs[N] = {
    {"Hub01_Central",         13.08, 80.27},
    {"Hub02_Egmore",          13.07, 80.26},
    {"Hub03_Parrys",          13.09, 80.29},
    {"Hub04_Royapettah",      13.05, 80.26},
    {"Hub05_Mylapore",        13.03, 80.27},
    {"Hub06_T_Nagar",         13.04, 80.23},
    {"Hub07_Nungambakkam",    13.06, 80.24},
    {"Hub08_Saidapet",        13.02, 80.22},
    {"Hub09_Guindy",          13.01, 80.21},
    {"Hub10_Adyar",           13.00, 80.25},
    {"Hub11_BesantNagar",     12.99, 80.26},
    {"Hub12_Velachery",       12.98, 80.22},
    {"Hub13_Thiruvanmiyur",   12.98, 80.25},
    {"Hub14_OMR_Perungudi",   12.96, 80.24},
    {"Hub15_Sholinganallur",  12.90, 80.23},
    {"Hub16_Airport",         12.98, 80.16},
    {"Hub17_Tambaram",        12.92, 80.11},
    {"Hub18_Porur",           13.03, 80.15},
    {"Hub19_Vadapalani",      13.05, 80.21},
    {"Hub20_Koyambedu",       13.07, 80.19}
};

/* Edge record for tree analysis */
typedef struct {
    int u, v, cost;
} CableEdge;

/* DSU for Kruskal verification */
static int dsu_find(int *parent, int i) {
    if (parent[i] != i) parent[i] = dsu_find(parent, parent[i]);
    return parent[i];
}

static bool dsu_union(int *parent, int *rank, int x, int y) {
    int rx = dsu_find(parent, x), ry = dsu_find(parent, y);
    if (rx == ry) return false;
    if (rank[rx] < rank[ry]) parent[rx] = ry;
    else if (rank[rx] > rank[ry]) parent[ry] = rx;
    else { parent[ry] = rx; rank[rx]++; }
    return true;
}

static int compare_edges(const void *a, const void *b) {
    return ((const CableEdge *)a)->cost - ((const CableEdge *)b)->cost;
}

/* Run Kruskal for independent cross-verification */
static int run_kruskal_verify(int cost_matrix[N][N], int forbidden_u, int forbidden_v) {
    CableEdge edges[N * N];
    int count = 0;
    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            if (cost_matrix[i][j] > 0) {
                if ((i == forbidden_u && j == forbidden_v) || (i == forbidden_v && j == forbidden_u)) {
                    continue;
                }
                edges[count++] = (CableEdge){i, j, cost_matrix[i][j]};
            }
        }
    }
    qsort(edges, (size_t)count, sizeof(CableEdge), compare_edges);

    int parent[N], rank[N] = {0};
    for (int i = 0; i < N; i++) parent[i] = i;

    int total = 0, added = 0;
    for (int i = 0; i < count; i++) {
        if (dsu_union(parent, rank, edges[i].u, edges[i].v)) {
            total += edges[i].cost;
            added++;
            if (added == N - 1) break;
        }
    }
    return (added == N - 1) ? total : -1;
}

/* Prim's MST implementation returning tree edges */
static int run_prim(int cost_matrix[N][N], int forbidden_u, int forbidden_v, CableEdge out_edges[N - 1]) {
    int key[N], parent[N], in_tree[N] = {0};
    for (int i = 0; i < N; i++) { key[i] = INT_MAX; parent[i] = -1; }
    key[0] = 0;

    int total_cost = 0;
    for (int step = 0; step < N; step++) {
        int u = -1;
        for (int v = 0; v < N; v++) {
            if (!in_tree[v] && (u == -1 || key[v] < key[u])) u = v;
        }
        if (u == -1 || key[u] == INT_MAX) return -1; /* Disconnected */
        in_tree[u] = 1;
        total_cost += key[u];

        for (int v = 0; v < N; v++) {
            if (!in_tree[v] && cost_matrix[u][v] > 0) {
                if ((u == forbidden_u && v == forbidden_v) || (u == forbidden_v && v == forbidden_u)) {
                    continue;
                }
                if (cost_matrix[u][v] < key[v]) {
                    key[v] = cost_matrix[u][v];
                    parent[v] = u;
                }
            }
        }
    }

    int edge_idx = 0;
    for (int v = 1; v < N; v++) {
        out_edges[edge_idx++] = (CableEdge){parent[v], v, cost_matrix[parent[v]][v]};
    }
    return total_cost;
}

int main(void) {
    printf("========================================================================================\n");
    printf("        B4.1: CHENNAI 20-HUB FIBRE CABLE LAYOUT PLANNING & FAULT SIMULATION             \n");
    printf("========================================================================================\n\n");

    int cost_matrix[N][N];
    memset(cost_matrix, 0, sizeof(cost_matrix));

    /* Build pairwise rounded Euclidean distances (~100 km per coordinate degree) */
    srand(42);
    int total_possible = 0, disallowed = 0;

    for (int i = 0; i < N; i++) {
        for (int j = i + 1; j < N; j++) {
            total_possible++;
            double dx = (hubs[i].x - hubs[j].x) * 111.0;
            double dy = (hubs[i].y - hubs[j].y) * 105.0;
            int dist_lakh = (int)(round(sqrt(dx * dx + dy * dy)) * 10.0);
            if (dist_lakh < 5) dist_lakh = 5;

            /* Disallow ~15% of pairs (e.g. crossing restricted zones, wetlands, ports) */
            if ((rand() % 100) < 15) {
                cost_matrix[i][j] = cost_matrix[j][i] = 0;
                disallowed++;
            } else {
                cost_matrix[i][j] = cost_matrix[j][i] = dist_lakh;
            }
        }
    }

    /* Ensure graph connectivity by adding a spine route if severed */
    for (int i = 0; i < N - 1; i++) {
        if (cost_matrix[i][i + 1] == 0) {
            double dx = (hubs[i].x - hubs[i + 1].x) * 111.0;
            double dy = (hubs[i].y - hubs[i + 1].y) * 105.0;
            cost_matrix[i][i + 1] = cost_matrix[i + 1][i] = (int)(round(sqrt(dx * dx + dy * dy)) * 10.0);
            disallowed--;
        }
    }

    printf("20 Hubs configured across Chennai metropolitan area.\n");
    printf("Allowed links: %d / %d (Disallowed ~15%%: %d routes forbidden due to terrain/zoning)\n\n",
           total_possible - disallowed, total_possible, disallowed);

    /* Phase 1: Initial MST Plan */
    CableEdge mst[N - 1];
    int base_total = run_prim(cost_matrix, -1, -1, mst);
    int kruskal_base = run_kruskal_verify(cost_matrix, -1, -1);

    printf("--- INITIAL FIBRE CABLE PLAN (Prim's Algorithm) ---\n");
    printf("Total Infrastructure Cost: %d lakh (Kruskal Verification: %d lakh - %s)\n",
           base_total, kruskal_base, base_total == kruskal_base ? "EXACT MATCH" : "ERROR");
    printf("%-4s | %-20s <-> %-20s | %s\n", "No", "Hub A", "Hub B", "Cost (Rs lakh)");
    printf("--------------------------------------------------------------------\n");

    int max_edge_idx = 0;
    for (int i = 0; i < N - 1; i++) {
        printf("%-4d | %-20s <-> %-20s | %d lakh\n",
               i + 1, hubs[mst[i].u].name, hubs[mst[i].v].name, mst[i].cost);
        if (mst[i].cost > mst[max_edge_idx].cost) {
            max_edge_idx = i;
        }
    }
    printf("--------------------------------------------------------------------\n");
    printf("Most Expensive Cable in MST: %s <-> %s (%d lakh)\n\n",
           hubs[mst[max_edge_idx].u].name, hubs[mst[max_edge_idx].v].name, mst[max_edge_idx].cost);

    /* Phase 2: Fault Simulation 1 — Sever Most Expensive MST Cable */
    CableEdge cut1 = mst[max_edge_idx];
    CableEdge mst_f1[N - 1];
    int total_f1 = run_prim(cost_matrix, cut1.u, cut1.v, mst_f1);
    int kruskal_f1 = run_kruskal_verify(cost_matrix, cut1.u, cut1.v);

    printf("--- FAULT SIMULATION 1: Sever Most Expensive Link [%s <-> %s] ---\n",
           hubs[cut1.u].name, hubs[cut1.v].name);
    printf("Re-routed MST Total Cost: %d lakh (Increase: +%d lakh)\n", total_f1, total_f1 - base_total);
    printf("Kruskal Cross-Check: %d lakh (%s)\n", kruskal_f1, total_f1 == kruskal_f1 ? "VERIFIED" : "ERROR");

    /* Identify replacement edge */
    printf("Replacement Edge Added to Restore Tree:\n");
    for (int i = 0; i < N - 1; i++) {
        bool in_old = false;
        for (int j = 0; j < N - 1; j++) {
            if ((mst_f1[i].u == mst[j].u && mst_f1[i].v == mst[j].v) ||
                (mst_f1[i].u == mst[j].v && mst_f1[i].v == mst[j].u)) {
                in_old = true; break;
            }
        }
        if (!in_old) {
            printf(" -> REPLACED WITH: %s <-> %s (Cost: %d lakh)\n",
                   hubs[mst_f1[i].u].name, hubs[mst_f1[i].v].name, mst_f1[i].cost);
        }
    }
    printf("\n");

    /* Phase 3: Fault Simulation 2 — Sever a Mid-Weight MST Cable */
    int mid_edge_idx = (N - 1) / 2;
    CableEdge cut2 = mst[mid_edge_idx];
    CableEdge mst_f2[N - 1];
    int total_f2 = run_prim(cost_matrix, cut2.u, cut2.v, mst_f2);
    int kruskal_f2 = run_kruskal_verify(cost_matrix, cut2.u, cut2.v);

    printf("--- FAULT SIMULATION 2: Sever Mid-Weight Link [%s <-> %s (%d lakh)] ---\n",
           hubs[cut2.u].name, hubs[cut2.v].name, cut2.cost);
    printf("Re-routed MST Total Cost: %d lakh (Increase: +%d lakh)\n", total_f2, total_f2 - base_total);
    printf("Kruskal Cross-Check: %d lakh (%s)\n", kruskal_f2, total_f2 == kruskal_f2 ? "VERIFIED" : "ERROR");

    printf("Replacement Edge Added to Restore Tree:\n");
    for (int i = 0; i < N - 1; i++) {
        bool in_old = false;
        for (int j = 0; j < N - 1; j++) {
            if ((mst_f2[i].u == mst[j].u && mst_f2[i].v == mst[j].v) ||
                (mst_f2[i].u == mst[j].v && mst_f2[i].v == mst[j].u)) {
                in_old = true; break;
            }
        }
        if (!in_old) {
            printf(" -> REPLACED WITH: %s <-> %s (Cost: %d lakh)\n",
                   hubs[mst_f2[i].u].name, hubs[mst_f2[i].v].name, mst_f2[i].cost);
        }
    }

    printf("========================================================================================\n");
    return 0;
}
