#include <stdio.h>
#define MAX 32
typedef struct { int severity; int patient; } Case;  /* higher = more urgent */
Case heap[MAX];
int size = 0;
void swap(Case *a, Case *b) { Case t = *a; *a = *b; *b = t; }
void push(int severity, int patient)
{
    if (size == MAX) return;
    heap[size] = (Case){severity, patient};
    int i = size++;
    // TODO 1: Sift up: while i is not the root and its parent (i-1)/2 has a smaller
    //   severity, swap them and move i to the parent
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap[parent].severity < heap[i].severity) {
            swap(&heap[parent], &heap[i]);
            i = parent;
        } else {
            break;
        }
    }
}
Case pop(void)
{
    Case top = heap[0];
    heap[0] = heap[--size];
    int i = 0;
    // TODO 2: Sift down: repeatedly swap i with its larger child (2i+1 or 2i+2) while
    //   that child has a greater severity
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
    return top;
}
int main(void)
{
    int sev[] = {3, 9, 5, 7, 1, 8};
    for (int p = 0; p < 6; p++) push(sev[p], 101 + p);
    while (size) {
        Case c = pop();
        printf("P%d(%d) ", c.patient, c.severity);
    }
    printf("\n");
    return 0;
}
