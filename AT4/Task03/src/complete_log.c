#include <stdio.h>

#define MAX 32
typedef struct { int severity; int patient; } Case;  /* higher = more urgent */
static Case heap[MAX];
static int size = 0;

static void swap(Case *a, Case *b) { Case t = *a; *a = *b; *b = t; }

static void print_heap_state(FILE *fp, const char *action)
{
    fprintf(fp, "%-32s -> Size %d: [ ", action, size);
    for (int i = 0; i < size; i++) {
        fprintf(fp, "P%d(%d) ", heap[i].patient, heap[i].severity);
    }
    fprintf(fp, "]\n");
}

static void push(int severity, int patient, FILE *fp)
{
    if (size == MAX) return;
    heap[size] = (Case){severity, patient};
    int i = size++;
    
    char msg[64];
    snprintf(msg, sizeof(msg), "Inserted P%d(sev %d)", patient, severity);
    
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap[parent].severity < heap[i].severity) {
            swap(&heap[parent], &heap[i]);
            i = parent;
        } else {
            break;
        }
    }
    print_heap_state(fp, msg);
}

static Case pop(FILE *fp)
{
    Case top = heap[0];
    heap[0] = heap[--size];
    int i = 0;
    while (2 * i + 1 < size) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int largest = i;
        if (left < size && heap[left].severity > heap[largest].severity) {
            largest = left;
        }
        if (right < size && heap[right].severity > heap[largest].severity) {
            largest = right;
        }
        if (largest == i) break;
        swap(&heap[i], &heap[largest]);
        i = largest;
    }
    char msg[64];
    snprintf(msg, sizeof(msg), "Popped top P%d(sev %d)", top.patient, top.severity);
    print_heap_state(fp, msg);
    return top;
}

int main(void)
{
    FILE *fp = fopen("Task03/output/heap_insertion_log.txt", "w");
    if (!fp) {
        perror("Failed to open heap_insertion_log.txt");
        return 1;
    }

    fprintf(fp, "================================================================================\n");
    fprintf(fp, "          HEAP ARRAY STATE AFTER EVERY INSERTION AND POP (MAX-HEAP)\n");
    fprintf(fp, "================================================================================\n\n");
    fprintf(fp, "--- INSERTION PHASE (Sift-Up) ---\n");

    int sev[] = {3, 9, 5, 7, 1, 8};
    for (int p = 0; p < 6; p++) {
        push(sev[p], 101 + p, fp);
    }

    fprintf(fp, "\n--- REMOVAL PHASE (Sift-Down / Pop-Max) ---\n");
    while (size) {
        pop(fp);
    }

    fclose(fp);
    printf("Heap trace log saved to Task03/output/heap_insertion_log.txt\n");
    return 0;
}
