#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Graph* graph_create(int initial_capacity) {
    if (initial_capacity < 16) initial_capacity = 16;
    Graph* g = (Graph*)malloc(sizeof(Graph));
    if (!g) return NULL;
    g->num_vertices = 0;
    g->capacity = initial_capacity;
    g->vertices = (Vertex*)calloc(g->capacity, sizeof(Vertex));
    if (!g->vertices) {
        free(g);
        return NULL;
    }
    return g;
}

void graph_free(Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->num_vertices; i++) {
        EdgeNode* curr = g->vertices[i].head;
        while (curr) {
            EdgeNode* next = curr->next;
            free(curr);
            curr = next;
        }
    }
    free(g->vertices);
    free(g);
}

int graph_find_vertex(const Graph* g, const char* name) {
    if (!g || !name) return -1;
    for (int i = 0; i < g->num_vertices; i++) {
        if (strcmp(g->vertices[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int graph_add_vertex(Graph* g, const char* name) {
    if (!g || !name) return -1;
    int idx = graph_find_vertex(g, name);
    if (idx >= 0) return idx;

    if (g->num_vertices >= g->capacity) {
        int new_cap = g->capacity * 2;
        Vertex* nv = (Vertex*)realloc(g->vertices, sizeof(Vertex) * new_cap);
        if (!nv) return -1;
        memset(nv + g->capacity, 0, sizeof(Vertex) * (new_cap - g->capacity));
        g->vertices = nv;
        g->capacity = new_cap;
    }

    int id = g->num_vertices++;
    g->vertices[id].id = id;
    strncpy(g->vertices[id].name, name, MAX_NAME_LEN - 1);
    g->vertices[id].name[MAX_NAME_LEN - 1] = '\0';
    g->vertices[id].degree = 0;
    g->vertices[id].head = NULL;
    g->vertices[id].active = true;
    return id;
}

bool graph_has_edge(const Graph* g, int u, int v) {
    if (!g || u < 0 || u >= g->num_vertices || v < 0 || v >= g->num_vertices) return false;
    EdgeNode* curr = g->vertices[u].head;
    while (curr) {
        if (curr->to == v && curr->active) return true;
        curr = curr->next;
    }
    return false;
}

static void add_half_edge(Graph* g, int from, int to, int cost) {
    EdgeNode* node = (EdgeNode*)malloc(sizeof(EdgeNode));
    node->to = to;
    node->cost = cost;
    node->active = true;
    node->next = g->vertices[from].head;
    g->vertices[from].head = node;
    g->vertices[from].degree++;
}

bool graph_add_edge(Graph* g, const char* name_a, const char* name_b, int cost) {
    if (!g || !name_a || !name_b) return false;
    if (strcmp(name_a, name_b) == 0) return false; // no self loops

    int u = graph_add_vertex(g, name_a);
    int v = graph_add_vertex(g, name_b);
    if (u < 0 || v < 0) return false;

    // Check if edge already exists
    EdgeNode* curr = g->vertices[u].head;
    while (curr) {
        if (curr->to == v) {
            // Update cost if already exists
            curr->cost = cost;
            // Also update reverse
            EdgeNode* r = g->vertices[v].head;
            while (r) {
                if (r->to == u) { r->cost = cost; break; }
                r = r->next;
            }
            return true;
        }
        curr = curr->next;
    }

    add_half_edge(g, u, v, cost);
    add_half_edge(g, v, u, cost);
    return true;
}

bool graph_remove_edge(Graph* g, const char* name_a, const char* name_b) {
    int u = graph_find_vertex(g, name_a);
    int v = graph_find_vertex(g, name_b);
    if (u < 0 || v < 0) return false;

    bool found = false;
    // Remove v from u's list
    EdgeNode** curr = &g->vertices[u].head;
    while (*curr) {
        if ((*curr)->to == v) {
            EdgeNode* tmp = *curr;
            *curr = (*curr)->next;
            free(tmp);
            g->vertices[u].degree--;
            found = true;
            break;
        }
        curr = &(*curr)->next;
    }

    // Remove u from v's list
    curr = &g->vertices[v].head;
    while (*curr) {
        if ((*curr)->to == u) {
            EdgeNode* tmp = *curr;
            *curr = (*curr)->next;
            free(tmp);
            g->vertices[v].degree--;
            break;
        }
        curr = &(*curr)->next;
    }

    return found;
}

int graph_get_active_degree(const Graph* g, int u) {
    if (!g || u < 0 || u >= g->num_vertices || !g->vertices[u].active) return 0;
    int deg = 0;
    EdgeNode* curr = g->vertices[u].head;
    while (curr) {
        if (curr->active && g->vertices[curr->to].active) {
            deg++;
        }
        curr = curr->next;
    }
    return deg;
}

void graph_sort_edges(Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->num_vertices; i++) {
        // Collect into array and sort by destination vertex name
        int count = 0;
        EdgeNode* curr = g->vertices[i].head;
        while (curr) { count++; curr = curr->next; }
        if (count <= 1) continue;

        EdgeNode** arr = (EdgeNode**)malloc(sizeof(EdgeNode*) * count);
        curr = g->vertices[i].head;
        for (int k = 0; k < count; k++) {
            arr[k] = curr;
            curr = curr->next;
        }

        // Insertion sort by name
        for (int j = 1; j < count; j++) {
            EdgeNode* key = arr[j];
            int p = j - 1;
            while (p >= 0 && strcmp(g->vertices[arr[p]->to].name, g->vertices[key->to].name) > 0) {
                arr[p + 1] = arr[p];
                p--;
            }
            arr[p + 1] = key;
        }

        // Re-link
        for (int k = 0; k < count - 1; k++) {
            arr[k]->next = arr[k + 1];
        }
        arr[count - 1]->next = NULL;
        g->vertices[i].head = arr[0];
        free(arr);
    }
}

static char* trim_whitespace(char* str) {
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

Graph* graph_load_from_file(const char* filepath) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) return NULL;

    Graph* g = graph_create(32);
    if (!g) {
        fclose(fp);
        return NULL;
    }

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        char* trimmed = trim_whitespace(line);
        if (*trimmed == '\0' || *trimmed == '#' || *trimmed == ';') {
            continue;
        }

        // Check for DEVICE <name>
        if (strncmp(trimmed, "DEVICE", 6) == 0 && isspace((unsigned char)trimmed[6])) {
            char name[64];
            if (sscanf(trimmed + 6, "%63s", name) == 1) {
                graph_add_vertex(g, name);
            }
            continue;
        }

        // Check for LINK <a> <b> <cost>
        if (strncmp(trimmed, "LINK", 4) == 0 && isspace((unsigned char)trimmed[4])) {
            char a[64], b[64];
            int cost = 1;
            int n = sscanf(trimmed + 4, "%63s %63s %d", a, b, &cost);
            if (n >= 2) {
                if (n == 2) cost = 1;
                graph_add_edge(g, a, b, cost);
            }
            continue;
        }

        // Generic token parsing: could be "A B 5" or "A B" or "A"
        char w1[64], w2[64], w3[64];
        int num = sscanf(trimmed, "%63s %63s %63s", w1, w2, w3);
        if (num == 1) {
            graph_add_vertex(g, w1);
        } else if (num == 2) {
            graph_add_edge(g, w1, w2, 1);
        } else if (num == 3) {
            int cost = atoi(w3);
            if (cost <= 0) cost = 1;
            graph_add_edge(g, w1, w2, cost);
        }
    }

    fclose(fp);
    graph_sort_edges(g);
    return g;
}

bool graph_save_to_file(const Graph* g, const char* filepath) {
    if (!g || !filepath) return false;
    FILE* fp = fopen(filepath, "w");
    if (!fp) return false;

    fprintf(fp, "# Network Topology File\n");
    fprintf(fp, "# Generated by Network Connectivity Analyzer\n\n");

    fprintf(fp, "# Devices\n");
    for (int i = 0; i < g->num_vertices; i++) {
        fprintf(fp, "DEVICE %s\n", g->vertices[i].name);
    }

    fprintf(fp, "\n# Links\n");
    for (int i = 0; i < g->num_vertices; i++) {
        EdgeNode* curr = g->vertices[i].head;
        while (curr) {
            // Print each undirected link once where i < curr->to
            if (i < curr->to) {
                fprintf(fp, "LINK %s %s %d\n", g->vertices[i].name, g->vertices[curr->to].name, curr->cost);
            }
            curr = curr->next;
        }
    }

    fclose(fp);
    return true;
}
