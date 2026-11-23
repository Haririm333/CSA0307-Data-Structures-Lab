#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

#define MAX_NAME_LEN 32

typedef struct EdgeNode {
    int to;
    int cost;
    bool active;
    struct EdgeNode* next;
} EdgeNode;

typedef struct Vertex {
    int id;
    char name[MAX_NAME_LEN];
    int degree;
    EdgeNode* head;
    bool active;
} Vertex;

typedef struct Graph {
    int num_vertices;
    int capacity;
    Vertex* vertices;
} Graph;

Graph* graph_create(int initial_capacity);
void graph_free(Graph* g);
int graph_add_vertex(Graph* g, const char* name);
int graph_find_vertex(const Graph* g, const char* name);
bool graph_add_edge(Graph* g, const char* name_a, const char* name_b, int cost);
bool graph_has_edge(const Graph* g, int u, int v);
bool graph_remove_edge(Graph* g, const char* name_a, const char* name_b);
Graph* graph_load_from_file(const char* filepath);
bool graph_save_to_file(const Graph* g, const char* filepath);
void graph_sort_edges(Graph* g);
int graph_get_active_degree(const Graph* g, int u);

#endif
