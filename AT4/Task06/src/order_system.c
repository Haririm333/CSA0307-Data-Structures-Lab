#include "order_system.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ==================== PRIME NUMBERS & HASH FUNCTION ==================== */
static bool is_prime(int n)
{
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

static int next_prime(int n)
{
    while (!is_prime(n)) n++;
    return n;
}

static inline unsigned hash_fnv1a_id(int id, int capacity)
{
    uint32_t h = 2166136261u;
    uint32_t k = (uint32_t)id;
    for (int b = 0; b < 4; b++) {
        h ^= (uint8_t)(k & 0xFF);
        h *= 16777619u;
        k >>= 8;
    }
    return (unsigned)(h % (uint32_t)capacity);
}

/* ==================== HASH TABLE IMPLEMENTATION ==================== */
static HashTable *hash_table_create(int initial_cap)
{
    HashTable *ht = malloc(sizeof(HashTable));
    ht->capacity = next_prime(initial_cap);
    ht->count = 0;
    ht->max_load_factor = 0.75;
    ht->buckets = calloc(ht->capacity, sizeof(HashNode *));
    return ht;
}

static void hash_table_rehash(HashTable *ht)
{
    int old_cap = ht->capacity;
    int new_cap = next_prime(old_cap * 2);
    HashNode **new_buckets = calloc(new_cap, sizeof(HashNode *));

    for (int i = 0; i < old_cap; i++) {
        HashNode *curr = ht->buckets[i];
        while (curr) {
            HashNode *next = curr->next;
            unsigned new_idx = hash_fnv1a_id(curr->order->id, new_cap);
            curr->next = new_buckets[new_idx];
            new_buckets[new_idx] = curr;
            curr = next;
        }
    }
    free(ht->buckets);
    ht->buckets = new_buckets;
    ht->capacity = new_cap;
}

static void hash_table_insert(HashTable *ht, Order *order)
{
    if ((double)(ht->count + 1) / ht->capacity > ht->max_load_factor) {
        hash_table_rehash(ht);
    }
    unsigned b = hash_fnv1a_id(order->id, ht->capacity);
    HashNode *n = malloc(sizeof(HashNode));
    n->order = order;
    n->next = ht->buckets[b];
    ht->buckets[b] = n;
    ht->count++;
}

static Order *hash_table_lookup(HashTable *ht, int id)
{
    unsigned b = hash_fnv1a_id(id, ht->capacity);
    for (HashNode *curr = ht->buckets[b]; curr; curr = curr->next) {
        if (curr->order->id == id) {
            return curr->order;
        }
    }
    return NULL;
}

static void hash_table_free(HashTable *ht)
{
    for (int i = 0; i < ht->capacity; i++) {
        HashNode *curr = ht->buckets[i];
        while (curr) {
            HashNode *tmp = curr->next;
            free(curr);
            curr = tmp;
        }
    }
    free(ht->buckets);
    free(ht);
}

/* ==================== INDEXED HEAP IMPLEMENTATION ==================== */
static IndexedHeap *heap_create(int capacity)
{
    IndexedHeap *h = malloc(sizeof(IndexedHeap));
    h->data = malloc(capacity * sizeof(Order *));
    h->size = 0;
    h->capacity = capacity;
    return h;
}

/* Returns true if 'a' has higher priority than 'b' */
static inline bool heap_higher_priority(const Order *a, const Order *b)
{
    if (a->delay_priority != b->delay_priority) {
        return a->delay_priority > b->delay_priority; /* More overdue = higher priority */
    }
    /* Tie-breaker: earlier creation time */
    return a->created_time < b->created_time;
}

static inline void heap_swap(IndexedHeap *h, int i, int j)
{
    Order *t = h->data[i];
    h->data[i] = h->data[j];
    h->data[j] = t;
    h->data[i]->heap_index = i;
    h->data[j]->heap_index = j;
}

static void heap_sift_up(IndexedHeap *h, int i)
{
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap_higher_priority(h->data[i], h->data[parent])) {
            heap_swap(h, i, parent);
            i = parent;
        } else {
            break;
        }
    }
}

static void heap_sift_down(IndexedHeap *h, int i)
{
    while (2 * i + 1 < h->size) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int best = i;

        if (left < h->size && heap_higher_priority(h->data[left], h->data[best])) {
            best = left;
        }
        if (right < h->size && heap_higher_priority(h->data[right], h->data[best])) {
            best = right;
        }
        if (best == i) break;
        heap_swap(h, i, best);
        i = best;
    }
}

static void heap_push(IndexedHeap *h, Order *order)
{
    if (h->size >= h->capacity) {
        h->capacity *= 2;
        h->data = realloc(h->data, h->capacity * sizeof(Order *));
    }
    int i = h->size++;
    h->data[i] = order;
    order->heap_index = i;
    heap_sift_up(h, i);
}

static Order *heap_pop_max(IndexedHeap *h)
{
    if (h->size == 0) return NULL;
    Order *top = h->data[0];
    top->heap_index = -1;

    h->size--;
    if (h->size > 0) {
        h->data[0] = h->data[h->size];
        h->data[0]->heap_index = 0;
        heap_sift_down(h, 0);
    }
    return top;
}

static bool heap_remove_at(IndexedHeap *h, int idx)
{
    if (idx < 0 || idx >= h->size) return false;
    Order *target = h->data[idx];
    target->heap_index = -1;

    h->size--;
    if (idx == h->size) {
        return true;
    }

    h->data[idx] = h->data[h->size];
    h->data[idx]->heap_index = idx;

    heap_sift_down(h, idx);
    heap_sift_up(h, idx);
    return true;
}

static void heap_free(IndexedHeap *h)
{
    free(h->data);
    free(h);
}

/* ==================== SYSTEM CORE API ==================== */
DeliverySystem *system_init(int initial_capacity)
{
    DeliverySystem *sys = malloc(sizeof(DeliverySystem));
    sys->capacity = initial_capacity;
    sys->total_orders_placed = 0;
    sys->all_orders = malloc(initial_capacity * sizeof(Order *));
    sys->hash_table = hash_table_create(initial_capacity);
    sys->heap = heap_create(initial_capacity);
    return sys;
}

void system_free(DeliverySystem *sys)
{
    for (int i = 0; i < sys->total_orders_placed; i++) {
        free(sys->all_orders[i]);
    }
    free(sys->all_orders);
    hash_table_free(sys->hash_table);
    heap_free(sys->heap);
    free(sys);
}

bool place_order(DeliverySystem *sys, int id, const char *restaurant, int amount,
                 long long created_time, long long promised_time)
{
    if (sys->total_orders_placed >= sys->capacity) {
        sys->capacity *= 2;
        sys->all_orders = realloc(sys->all_orders, sys->capacity * sizeof(Order *));
    }

    Order *o = malloc(sizeof(Order));
    o->id = id;
    strncpy(o->restaurant, restaurant, sizeof(o->restaurant) - 1);
    o->restaurant[sizeof(o->restaurant) - 1] = '\0';
    o->amount = amount;
    o->created_time = created_time;
    o->promised_time = promised_time;
    /* Invariant priority: earlier promised time represents higher delay */
    o->delay_priority = -promised_time;
    o->status = ORDER_PENDING;
    o->heap_index = -1;

    sys->all_orders[sys->total_orders_placed++] = o;

    /* 1. Insert into Hash Table for O(1) lookup */
    hash_table_insert(sys->hash_table, o);

    /* 2. Insert into Indexed Binary Max-Heap in O(log n) */
    heap_push(sys->heap, o);

    return true;
}

Order *lookup(DeliverySystem *sys, int id)
{
    return hash_table_lookup(sys->hash_table, id);
}

Order *assign_next_rider(DeliverySystem *sys, long long current_time)
{
    Order *o = heap_pop_max(sys->heap);
    if (!o) return NULL;
    o->status = ORDER_ASSIGNED;
    /* Refresh exact overdue delay at assignment time */
    o->delay_priority = current_time - o->promised_time;
    return o;
}

bool cancel_order(DeliverySystem *sys, int id)
{
    Order *o = hash_table_lookup(sys->hash_table, id);
    if (!o) return false;
    if (o->status != ORDER_PENDING) return false;

    int idx = o->heap_index;
    if (idx >= 0) {
        heap_remove_at(sys->heap, idx);
    }
    o->status = ORDER_CANCELLED;
    return true;
}

/* ==================== STABLE MERGE SORT REPORT ==================== */
static bool report_compare_less_equal(const Order *a, const Order *b)
{
    int cmp = strcmp(a->restaurant, b->restaurant);
    if (cmp < 0) return true;
    if (cmp > 0) return false;

    /* Same restaurant: order by amount */
    if (a->amount < b->amount) return true;
    if (a->amount > b->amount) return false;

    /* Equal amounts: '<=' guarantees original arrival/created_time order is strictly maintained */
    return true;
}

static void merge_report(Order *a[], Order *tmp[], int lo, int mid, int hi)
{
    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi) {
        if (report_compare_less_equal(a[i], a[j])) {
            tmp[k++] = a[i++];
        } else {
            tmp[k++] = a[j++];
        }
    }
    while (i <= mid) tmp[k++] = a[i++];
    while (j <= hi)  tmp[k++] = a[j++];
    for (int p = lo; p <= hi; p++) a[p] = tmp[p];
}

static void merge_sort_report(Order *a[], Order *tmp[], int lo, int hi)
{
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort_report(a, tmp, lo, mid);
    merge_sort_report(a, tmp, mid + 1, hi);
    merge_report(a, tmp, lo, mid, hi);
}

void hourly_report(DeliverySystem *sys, Order *output_array[], int *report_count)
{
    int n = sys->total_orders_placed;
    *report_count = n;
    for (int i = 0; i < n; i++) {
        output_array[i] = sys->all_orders[i];
    }
    Order **tmp = malloc(n * sizeof(Order *));
    merge_sort_report(output_array, tmp, 0, n - 1);
    free(tmp);
}
