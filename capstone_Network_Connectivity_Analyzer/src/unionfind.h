#ifndef UNIONFIND_H
#define UNIONFIND_H

#include "graph.h"
#include <stdbool.h>

typedef struct UnionFind {
    int n;
    int* parent;
    int* rank;
} UnionFind;

typedef struct ComponentGroup {
    int* vertex_ids;
    int size;
} ComponentGroup;

typedef struct ComponentList {
    ComponentGroup* groups;
    int count;
} ComponentList;

UnionFind* uf_create(int n);
void uf_free(UnionFind* uf);
int uf_find(UnionFind* uf, int i);
bool uf_union(UnionFind* uf, int i, int j);
bool uf_connected(UnionFind* uf, int i, int j);

UnionFind* uf_from_graph(const Graph* g);
ComponentList* uf_get_components(const Graph* g);
void component_list_free(ComponentList* list);

#endif
