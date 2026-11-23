#include "faults.h"
#include <stdlib.h>
#include <string.h>

static int min_int(int a, int b) { return a < b ? a : b; }

static void bridge_dfs(const Graph* g, int u, int p, bool* visited, int* tin, int* low, int* timer,
                       Bridge* bridges, int* bridge_count, bool* is_ap) {
    visited[u] = true;
    tin[u] = low[u] = ++(*timer);
    int children = 0;

    EdgeNode* edge = g->vertices[u].head;
    while (edge) {
        int v = edge->to;
        if (!edge->active || !g->vertices[v].active) {
            edge = edge->next;
            continue;
        }

        if (v == p) {
            edge = edge->next;
            continue;
        }

        if (visited[v]) {
            low[u] = min_int(low[u], tin[v]);
        } else {
            children++;
            bridge_dfs(g, v, u, visited, tin, low, timer, bridges, bridge_count, is_ap);
            low[u] = min_int(low[u], low[v]);

            // Bridge condition
            if (low[v] > tin[u]) {
                bridges[(*bridge_count)++] = (Bridge){ u, v };
            }

            // Articulation point condition for non-root
            if (p != -1 && low[v] >= tin[u]) {
                is_ap[u] = true;
            }
        }
        edge = edge->next;
    }

    // Articulation point condition for root
    if (p == -1 && children > 1) {
        is_ap[u] = true;
    }
}

FaultResult* faults_detect(const Graph* g) {
    if (!g) return NULL;
    int n = g->num_vertices;

    FaultResult* res = (FaultResult*)malloc(sizeof(FaultResult));
    res->bridges = (Bridge*)malloc(sizeof(Bridge) * (n * 10 + 16));
    res->bridge_count = 0;
    res->articulation_points = (int*)malloc(sizeof(int) * n);
    res->articulation_count = 0;

    bool* visited = (bool*)calloc(n, sizeof(bool));
    int* tin = (int*)malloc(sizeof(int) * n);
    int* low = (int*)malloc(sizeof(int) * n);
    bool* is_ap = (bool*)calloc(n, sizeof(bool));
    int timer = 0;

    for (int i = 0; i < n; i++) {
        if (g->vertices[i].active && !visited[i]) {
            bridge_dfs(g, i, -1, visited, tin, low, &timer, res->bridges, &res->bridge_count, is_ap);
        }
    }

    // Collect articulation points
    for (int i = 0; i < n; i++) {
        if (is_ap[i] && g->vertices[i].active) {
            res->articulation_points[res->articulation_count++] = i;
        }
    }

    // Canonicalize bridge vertex order (name alphabetically)
    for (int i = 0; i < res->bridge_count; i++) {
        int u = res->bridges[i].u;
        int v = res->bridges[i].v;
        if (strcmp(g->vertices[u].name, g->vertices[v].name) > 0) {
            res->bridges[i].u = v;
            res->bridges[i].v = u;
        }
    }

    // Sort bridges
    for (int i = 0; i < res->bridge_count - 1; i++) {
        for (int j = i + 1; j < res->bridge_count; j++) {
            const char* au = g->vertices[res->bridges[i].u].name;
            const char* bu = g->vertices[res->bridges[j].u].name;
            int cmp = strcmp(au, bu);
            if (cmp > 0 || (cmp == 0 && strcmp(g->vertices[res->bridges[i].v].name, g->vertices[res->bridges[j].v].name) > 0)) {
                Bridge tmp = res->bridges[i];
                res->bridges[i] = res->bridges[j];
                res->bridges[j] = tmp;
            }
        }
    }

    // Sort articulation points
    for (int i = 0; i < res->articulation_count - 1; i++) {
        for (int j = i + 1; j < res->articulation_count; j++) {
            if (strcmp(g->vertices[res->articulation_points[i]].name, g->vertices[res->articulation_points[j]].name) > 0) {
                int tmp = res->articulation_points[i];
                res->articulation_points[i] = res->articulation_points[j];
                res->articulation_points[j] = tmp;
            }
        }
    }

    free(visited);
    free(tin);
    free(low);
    free(is_ap);
    return res;
}

void fault_result_free(FaultResult* res) {
    if (!res) return;
    if (res->bridges) free(res->bridges);
    if (res->articulation_points) free(res->articulation_points);
    free(res);
}

SimResult* faults_simulate_link(Graph* g, const char* name_a, const char* name_b) {
    if (!g || !name_a || !name_b) return NULL;
    int u = graph_find_vertex(g, name_a);
    int v = graph_find_vertex(g, name_b);
    if (u < 0 || v < 0) return NULL;

    ComponentList* before = uf_get_components(g);
    int comp_before = before ? before->count : 0;
    component_list_free(before);

    // Deactivate edge
    EdgeNode* edge = g->vertices[u].head;
    while (edge) {
        if (edge->to == v) edge->active = false;
        edge = edge->next;
    }
    edge = g->vertices[v].head;
    while (edge) {
        if (edge->to == u) edge->active = false;
        edge = edge->next;
    }

    ComponentList* after = uf_get_components(g);
    int comp_after = after ? after->count : 0;

    // Restore edge
    edge = g->vertices[u].head;
    while (edge) {
        if (edge->to == v) edge->active = true;
        edge = edge->next;
    }
    edge = g->vertices[v].head;
    while (edge) {
        if (edge->to == u) edge->active = true;
        edge = edge->next;
    }

    SimResult* sim = (SimResult*)malloc(sizeof(SimResult));
    sim->components_before = comp_before;
    sim->components_after = comp_after;
    sim->disconnected_groups = after;
    return sim;
}

SimResult* faults_simulate_device(Graph* g, const char* name) {
    if (!g || !name) return NULL;
    int u = graph_find_vertex(g, name);
    if (u < 0) return NULL;

    ComponentList* before = uf_get_components(g);
    int comp_before = before ? before->count : 0;
    component_list_free(before);

    // Deactivate vertex
    g->vertices[u].active = false;

    ComponentList* after = uf_get_components(g);
    int comp_after = after ? after->count : 0;

    // Restore vertex
    g->vertices[u].active = true;

    SimResult* sim = (SimResult*)malloc(sizeof(SimResult));
    sim->components_before = comp_before;
    sim->components_after = comp_after;
    sim->disconnected_groups = after;
    return sim;
}

void sim_result_free(SimResult* res) {
    if (!res) return;
    if (res->disconnected_groups) {
        component_list_free(res->disconnected_groups);
    }
    free(res);
}
