#include <stdio.h>
#include <string.h>
typedef struct { int order_id; int amount; } Order;
long comparisons = 0;

/* Using strictly '<' causes instability: when a[i].amount == a[j].amount,
   it takes the right half (a[j]) first, reversing the relative order! */
void merge(Order a[], Order tmp[], int lo, int mid, int hi)
{
    int i = lo;
    int j = mid + 1;
    int k = lo;

    while (i <= mid && j <= hi) {
        comparisons++;
        if (a[i].amount < a[j].amount) { /* UNSTABLE: '<' instead of '<=' */
            tmp[k++] = a[i++];
        } else {
            tmp[k++] = a[j++];
        }
    }
    while (i <= mid) {
        tmp[k++] = a[i++];
    }
    while (j <= hi) {
        tmp[k++] = a[j++];
    }
    for (int p = lo; p <= hi; p++) {
        a[p] = tmp[p];
    }
}

void merge_sort(Order a[], Order tmp[], int lo, int hi)
{
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort(a, tmp, lo, mid);
    merge_sort(a, tmp, mid + 1, hi);
    merge(a, tmp, lo, mid, hi);
}

int main(void)
{
    Order o[8] = {{1, 450}, {2, 120}, {3, 450}, {4, 990},
                  {5, 120}, {6, 300}, {7, 780}, {8, 300}};
    Order tmp[8];
    merge_sort(o, tmp, 0, 7);
    for (int i = 0; i < 8; i++) printf("#%d:%d ", o[i].order_id, o[i].amount);
    printf("\ncomparisons = %ld\n", comparisons);
    return 0;
}
