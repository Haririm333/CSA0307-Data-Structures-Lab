#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double get_time_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

/* ==================== 1. BINARY HEAP ==================== */
typedef struct {
    int *data;
    int size;
    int capacity;
} BinaryHeap;

static BinaryHeap *create_heap(int cap)
{
    BinaryHeap *h = malloc(sizeof(BinaryHeap));
    h->data = malloc(cap * sizeof(int));
    h->size = 0;
    h->capacity = cap;
    return h;
}

static void heap_push(BinaryHeap *h, int val)
{
    int i = h->size++;
    h->data[i] = val;
    while (i > 0) {
        int p = (i - 1) / 2;
        if (h->data[i] > h->data[p]) {
            int t = h->data[i]; h->data[i] = h->data[p]; h->data[p] = t;
            i = p;
        } else {
            break;
        }
    }
}

static int heap_pop(BinaryHeap *h)
{
    int top = h->data[0];
    h->data[0] = h->data[--h->size];
    int i = 0;
    while (2 * i + 1 < h->size) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int largest = i;
        if (left < h->size && h->data[left] > h->data[largest]) largest = left;
        if (right < h->size && h->data[right] > h->data[largest]) largest = right;
        if (largest == i) break;
        int t = h->data[i]; h->data[i] = h->data[largest]; h->data[largest] = t;
        i = largest;
    }
    return top;
}

static void free_heap(BinaryHeap *h)
{
    free(h->data);
    free(h);
}

/* ==================== 2. SORTED LINKED LIST PQ ==================== */
typedef struct LNode {
    int val;
    struct LNode *next;
} LNode;

typedef struct {
    LNode *head;
    LNode *free_pool; /* fast node pool allocator to minimize malloc overhead */
    size_t pool_idx;
    LNode *pool_mem;
} SortedListPQ;

static SortedListPQ *create_list_pq(int max_nodes)
{
    SortedListPQ *pq = malloc(sizeof(SortedListPQ));
    pq->head = NULL;
    pq->pool_mem = malloc(max_nodes * sizeof(LNode));
    pq->pool_idx = 0;
    return pq;
}

static void list_push(SortedListPQ *pq, int val)
{
    LNode *n = &pq->pool_mem[pq->pool_idx++];
    n->val = val;
    n->next = NULL;

    /* Insert in descending order so highest is always at head */
    if (!pq->head || val >= pq->head->val) {
        n->next = pq->head;
        pq->head = n;
        return;
    }

    LNode *curr = pq->head;
    while (curr->next && curr->next->val > val) {
        curr = curr->next;
    }
    n->next = curr->next;
    curr->next = n;
}

static int list_pop(SortedListPQ *pq)
{
    LNode *top = pq->head;
    int val = top->val;
    pq->head = top->next;
    return val;
}

static void free_list_pq(SortedListPQ *pq)
{
    free(pq->pool_mem);
    free(pq);
}

int main(void)
{
    int test_sizes[] = {1000, 5000, 10000, 20000, 30000, 50000};
    int n_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);

    FILE *csv = fopen("Task03/output/pq_comparison.csv", "w");
    if (!csv) {
        perror("Failed to open pq_comparison.csv");
        return 1;
    }
    fprintf(csv, "N,Heap_Insert_Ms,Heap_Pop_Ms,Heap_Total_Ms,List_Insert_Ms,List_Pop_Ms,List_Total_Ms,Speedup_HeapVsList\n");

    printf("=========================================================================================================\n");
    printf("         PRIORITY QUEUE BENCHMARK: BINARY HEAP (O(log n)) vs SORTED LIST (O(n) Insert)\n");
    printf("=========================================================================================================\n");
    printf("%-7s | %-12s %-12s %-12s | %-12s %-12s %-12s | %-8s\n",
           "N", "Heap Ins(ms)", "Heap Pop(ms)", "Heap Tot(ms)", "List Ins(ms)", "List Pop(ms)", "List Tot(ms)", "Speedup");
    printf("---------------------------------------------------------------------------------------------------------\n");

    srand(777);

    for (int t = 0; t < n_tests; t++) {
        int n = test_sizes[t];
        int *data = malloc(n * sizeof(int));
        for (int i = 0; i < n; i++) data[i] = rand() % 100000;

        /* 1. Benchmark Binary Heap */
        BinaryHeap *h = create_heap(n);
        double t0 = get_time_sec();
        for (int i = 0; i < n; i++) heap_push(h, data[i]);
        double t1 = get_time_sec();
        volatile int dummy = 0;
        for (int i = 0; i < n; i++) dummy += heap_pop(h);
        double t2 = get_time_sec();

        double heap_ins_ms = (t1 - t0) * 1000.0;
        double heap_pop_ms = (t2 - t1) * 1000.0;
        double heap_tot_ms = (t2 - t0) * 1000.0;
        free_heap(h);

        /* 2. Benchmark Sorted Linked List */
        SortedListPQ *lpq = create_list_pq(n);
        t0 = get_time_sec();
        for (int i = 0; i < n; i++) list_push(lpq, data[i]);
        t1 = get_time_sec();
        for (int i = 0; i < n; i++) dummy += list_pop(lpq);
        t2 = get_time_sec();
        (void)dummy;

        double list_ins_ms = (t1 - t0) * 1000.0;
        double list_pop_ms = (t2 - t1) * 1000.0;
        double list_tot_ms = (t2 - t0) * 1000.0;
        free_list_pq(lpq);

        double speedup = list_tot_ms / (heap_tot_ms > 0.0001 ? heap_tot_ms : 0.0001);

        printf("%-7d | %-12.2f %-12.2f %-12.2f | %-12.2f %-12.2f %-12.2f | %-8.1fx\n",
               n, heap_ins_ms, heap_pop_ms, heap_tot_ms, list_ins_ms, list_pop_ms, list_tot_ms, speedup);

        fprintf(csv, "%d,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.1f\n",
                n, heap_ins_ms, heap_pop_ms, heap_tot_ms, list_ins_ms, list_pop_ms, list_tot_ms, speedup);

        free(data);
    }
    printf("=========================================================================================================\n");

    fclose(csv);
    printf("Priority queue benchmark results saved to Task03/output/pq_comparison.csv\n");
    return 0;
}
