#ifndef ORDER_SYSTEM_H
#define ORDER_SYSTEM_H

#include <stdbool.h>
#include <stdio.h>

typedef enum {
    ORDER_PENDING = 0,
    ORDER_ASSIGNED,
    ORDER_CANCELLED
} OrderStatus;

typedef struct Order {
    int id;
    char restaurant[32];
    int amount;
    long long created_time;    /* Simulation timestamp in seconds */
    long long promised_time;   /* Promised delivery deadline */
    long long delay_priority;  /* overdue delay: higher = more urgent */
    OrderStatus status;
    int heap_index;            /* Position in binary max-heap (-1 if not in heap) */
} Order;

/* Hash Table Node for separate chaining */
typedef struct HashNode {
    Order *order;
    struct HashNode *next;
} HashNode;

/* Dynamic Hash Table */
typedef struct {
    int capacity;
    int count;
    double max_load_factor;
    HashNode **buckets;
} HashTable;

/* Indexed Binary Max-Heap */
typedef struct {
    Order **data;
    int size;
    int capacity;
} IndexedHeap;

/* Food Delivery Platform System */
typedef struct {
    HashTable *hash_table;
    IndexedHeap *heap;
    Order **all_orders;
    int total_orders_placed;
    int capacity;
} DeliverySystem;

/* Core System Operations */
DeliverySystem *system_init(int initial_capacity);
void system_free(DeliverySystem *sys);

bool place_order(DeliverySystem *sys, int id, const char *restaurant, int amount,
                 long long created_time, long long promised_time);

Order *lookup(DeliverySystem *sys, int id);

Order *assign_next_rider(DeliverySystem *sys, long long current_time);

bool cancel_order(DeliverySystem *sys, int id);

void hourly_report(DeliverySystem *sys, Order *output_array[], int *report_count);

#endif /* ORDER_SYSTEM_H */
