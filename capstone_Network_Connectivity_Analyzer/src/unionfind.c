#include "unionfind.h"
#include <stdlib.h>
#include <string.h>

UnionFind* uf_create(int n) {
    UnionFind* uf = (UnionFind*)malloc(sizeof(UnionFind));
    uf->n = n;
    uf->parent = (int*)malloc(sizeof(int) * n);
    uf->rank = (int*)calloc(n, sizeof(int));
    for (int i = 0; i < n; i++) {
        uf->parent[i] = i;
    }
    return uf;
}

void uf_free(UnionFind* uf) {
    if (!uf) return;
    free(uf->parent);
    free(uf->rank);
    free(uf);
}

int uf_find(UnionFind* uf, int i) {
    if (uf->parent[i] != i) {
        uf->parent[i] = uf_find(uf, uf->parent[i]);
    }
    return uf->parent[i];
}

bool uf_union(UnionFind* uf, int i, int j) {
    int root_i = uf_find(uf, i);
    int root_j = uf_find(uf, j);
    if (root_i == root_j) return false;

    if (uf->rank[root_i] < uf->rank[root_j]) {
        uf->parent[root_i] = root_j;
    } else if (uf->rank[root_i] > uf->rank[root_j]) {
        uf->parent[root_j] = root_i;
    } else {
        uf->parent[root_j] = root_i;
        uf->rank[root_i]++;
    }
    return true;
}

bool uf_connected(UnionFind* uf, int i, int j) {
    if (!uf || i < 0 || i >= uf->n || j < 0 || j >= uf->n) return false;
    return uf_find(uf, i) == uf_find(uf, j);
}

UnionFind* uf_from_graph(const Graph* g) {
    if (!g) return NULL;
    UnionFind* uf = uf_create(g->num_vertices);
    for (int u = 0; u < g->num_vertices; u++) {
        if (!g->vertices[u].active) continue;
        EdgeNode* edge = g->vertices[u].head;
        while (edge) {
            int v = edge->to;
            if (edge->active && g->vertices[v].active && u < v) {
                uf_union(uf, u, v);
            }
            edge = edge->next;
        }
    }
    return uf;
}

ComponentList* uf_get_components(const Graph* g) {
    if (!g) return NULL;
    UnionFind* uf = uf_from_graph(g);
    int n = g->num_vertices;

    int* root_map = (int*)malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) root_map[i] = -1;

    int comp_count = 0;
    for (int i = 0; i < n; i++) {
        if (!g->vertices[i].active) continue;
        int r = uf_find(uf, i);
        if (root_map[r] == -1) {
            root_map[r] = comp_count++;
        }
    }

    ComponentList* list = (ComponentList*)malloc(sizeof(ComponentList));
    list->count = comp_count;
    list->groups = (ComponentGroup*)calloc(comp_count, sizeof(ComponentGroup));

    // First count sizes
    for (int i = 0; i < n; i++) {
        if (!g->vertices[i].active) continue;
        int r = uf_find(uf, i);
        int grp_idx = root_map[r];
        list->groups[grp_idx].size++;
    }

    // Allocate arrays
    for (int k = 0; k < comp_count; k++) {
        list->groups[k].vertex_ids = (int*)malloc(sizeof(int) * list->groups[k].size);
        list->groups[k].size = 0; // reset to use as append counter
    }

    // Populate
    for (int i = 0; i < n; i++) {
        if (!g->vertices[i].active) continue;
        int r = uf_find(uf, i);
        int grp_idx = root_map[r];
        list->groups[grp_idx].vertex_ids[list->groups[grp_idx].size++] = i;
    }

    free(root_map);
    uf_free(uf);
    return list;
}

void component_list_free(ComponentList* list) {
    if (!list) return;
    for (int i = 0; i < list->count; i++) {
        if (list->groups[i].vertex_ids) {
            free(list->groups[i].vertex_ids);
        }
    }
    free(list->groups);
    free(list);
}
