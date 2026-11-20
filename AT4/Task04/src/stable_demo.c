#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    int roll_no;
    int marks; /* 0 to 100 */
} Student;

static void swap(Student *a, Student *b)
{
    Student t = *a; *a = *b; *b = t;
}

/* 1. Bad Quicksort on Students */
static void bad_quick(Student a[], int lo, int hi)
{
    if (lo >= hi) return;
    int p = a[hi].marks;
    int i = lo - 1;

    for (int j = lo; j < hi; j++) {
        if (a[j].marks <= p) {
            i++;
            swap(&a[i], &a[j]);
        }
    }
    swap(&a[i + 1], &a[hi]);
    bad_quick(a, lo, i);
    bad_quick(a, i + 2, hi);
}

/* 2. Merge Sort (Stable using <=) */
static void merge(Student a[], Student tmp[], int lo, int mid, int hi)
{
    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi) {
        if (a[i].marks <= a[j].marks) { /* <= preserves stability */
            tmp[k++] = a[i++];
        } else {
            tmp[k++] = a[j++];
        }
    }
    while (i <= mid) tmp[k++] = a[i++];
    while (j <= hi)  tmp[k++] = a[j++];
    for (int p = lo; p <= hi; p++) a[p] = tmp[p];
}

static void merge_sort(Student a[], Student tmp[], int lo, int hi)
{
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;
    merge_sort(a, tmp, lo, mid);
    merge_sort(a, tmp, mid + 1, hi);
    merge(a, tmp, lo, mid, hi);
}

/* 3. Counting Sort (Stable, marks 0..100) */
static void counting_sort(Student a[], int n)
{
    int count[101] = {0};
    Student *out = malloc(n * sizeof(Student));

    for (int i = 0; i < n; i++) {
        count[a[i].marks]++;
    }
    for (int m = 1; m <= 100; m++) {
        count[m] += count[m - 1];
    }
    /* Iterate backward to ensure stability */
    for (int i = n - 1; i >= 0; i--) {
        int m = a[i].marks;
        out[--count[m]] = a[i];
    }
    for (int i = 0; i < n; i++) {
        a[i] = out[i];
    }
    free(out);
}

static void print_students(FILE *fp, const Student a[], int n, const char *title)
{
    fprintf(fp, "%-30s: [ ", title);
    for (int i = 0; i < n; i++) {
        fprintf(fp, "#%d(m:%d) ", a[i].roll_no, a[i].marks);
    }
    fprintf(fp, "]\n");
}

int main(void)
{
    Student orig[] = {
        {101, 85},  /* 1st with 85 */
        {102, 92},  /* 1st with 92 */
        {103, 70},  /* only 70 */
        {104, 85},  /* 2nd with 85 */
        {105, 92},  /* 2nd with 92 */
        {106, 60},  /* only 60 */
        {107, 85},  /* 3rd with 85 */
        {108, 92}   /* 3rd with 92 */
    };
    int n = sizeof(orig) / sizeof(orig[0]);

    FILE *fp = fopen("Task04/output/stability_demo.txt", "w");
    if (!fp) {
        perror("Failed to open stability_demo.txt");
        return 1;
    }

    fprintf(fp, "================================================================================\n");
    fprintf(fp, "        STABILITY FORENSIC INVESTIGATION: EQUAL MARKS TIE RESOLUTION\n");
    fprintf(fp, "================================================================================\n\n");
    print_students(fp, orig, n, "Original Student Records");
    fprintf(fp, "Notice original order for marks 85: #101 -> #104 -> #107\n");
    fprintf(fp, "Notice original order for marks 92: #102 -> #105 -> #108\n\n");

    /* 1. Test Bad Quicksort */
    Student a_quick[8];
    memcpy(a_quick, orig, sizeof(orig));
    bad_quick(a_quick, 0, n - 1);
    print_students(fp, a_quick, n, "1. Bad Quicksort Output");

    /* 2. Test Merge Sort */
    Student a_merge[8], tmp[8];
    memcpy(a_merge, orig, sizeof(orig));
    merge_sort(a_merge, tmp, 0, n - 1);
    print_students(fp, a_merge, n, "2. Merge Sort Output (Stable)");

    /* 3. Test Counting Sort */
    Student a_count[8];
    memcpy(a_count, orig, sizeof(orig));
    counting_sort(a_count, n);
    print_students(fp, a_count, n, "3. Counting Sort Output (Stable)");

    fprintf(fp, "\n--------------------------------------------------------------------------------\n");
    fprintf(fp, "FORENSIC VERDICT:\n");
    fprintf(fp, "- Bad Quicksort swaps distant elements around pivot, scrambling relative student order.\n");
    fprintf(fp, "  Marks 85 students ended in scrambled order (e.g. #107 or #104 displaced).\n");
    fprintf(fp, "- Merge Sort with '<=' comparison guarantees that left-subarray elements precede right\n");
    fprintf(fp, "  elements on equal values, preserving exact arrival order: #101 -> #104 -> #107.\n");
    fprintf(fp, "- Counting Sort traversed from back to front placing elements at --count[m], maintaining\n");
    fprintf(fp, "  flawless FIFO stability: #101 -> #104 -> #107 and #102 -> #105 -> #108.\n");

    fclose(fp);
    printf("Stability demo complete. Output saved to Task04/output/stability_demo.txt\n");
    return 0;
}
