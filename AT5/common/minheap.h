#ifndef MINHEAP_H
#define MINHEAP_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    int v;
    long long dist;
} HeapNode;

typedef struct {
    HeapNode *data;
    int size;
    int capacity;
} MinHeap;

static inline MinHeap *minheap_create(int capacity) {
    MinHeap *h = (MinHeap *)malloc(sizeof(MinHeap));
    if (!h) return NULL;
    h->capacity = capacity > 16 ? capacity : 16;
    h->size = 0;
    h->data = (HeapNode *)malloc(sizeof(HeapNode) * (size_t)h->capacity);
    if (!h->data) {
        free(h);
        return NULL;
    }
    return h;
}

static inline void minheap_free(MinHeap *h) {
    if (h) {
        free(h->data);
        free(h);
    }
}

static inline void minheap_swap(HeapNode *a, HeapNode *b) {
    HeapNode tmp = *a;
    *a = *b;
    *b = tmp;
}

static inline void minheap_push(MinHeap *h, int v, long long dist) {
    if (h->size >= h->capacity) {
        int new_cap = h->capacity * 2;
        HeapNode *new_data = (HeapNode *)realloc(h->data, sizeof(HeapNode) * (size_t)new_cap);
        if (!new_data) return;
        h->data = new_data;
        h->capacity = new_cap;
    }
    int i = h->size++;
    h->data[i].v = v;
    h->data[i].dist = dist;
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (h->data[parent].dist <= h->data[i].dist) break;
        minheap_swap(&h->data[parent], &h->data[i]);
        i = parent;
    }
}

static inline bool minheap_pop(MinHeap *h, int *v, long long *dist) {
    if (h->size <= 0) return false;
    *v = h->data[0].v;
    *dist = h->data[0].dist;
    h->data[0] = h->data[--h->size];
    int i = 0;
    while (1) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;
        if (left < h->size && h->data[left].dist < h->data[smallest].dist) smallest = left;
        if (right < h->size && h->data[right].dist < h->data[smallest].dist) smallest = right;
        if (smallest == i) break;
        minheap_swap(&h->data[i], &h->data[smallest]);
        i = smallest;
    }
    return true;
}

static inline bool minheap_is_empty(const MinHeap *h) {
    return h == NULL || h->size == 0;
}

#endif /* MINHEAP_H */
