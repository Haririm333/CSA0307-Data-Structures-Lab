#include <stdio.h>
#include <stdbool.h>

#define MAX 64

typedef struct {
    int severity;      /* Higher = more urgent */
    int seq_num;       /* Arrival counter: lower = arrived earlier */
    int patient_id;
    const char *name;
} PatientCase;

static PatientCase heap[MAX];
static int size = 0;
static int global_arrival_counter = 0;

/* Returns true if 'a' has strictly higher priority than 'b' */
static bool higher_priority(const PatientCase *a, const PatientCase *b)
{
    if (a->severity != b->severity) {
        return a->severity > b->severity;
    }
    /* Tie-breaker: earlier arrival (lower seq_num) gets higher priority (FIFO fairness) */
    return a->seq_num < b->seq_num;
}

static void swap(PatientCase *a, PatientCase *b)
{
    PatientCase t = *a; *a = *b; *b = t;
}

static void push(int severity, int patient_id, const char *name)
{
    if (size >= MAX) return;
    PatientCase c;
    c.severity = severity;
    c.seq_num = ++global_arrival_counter;
    c.patient_id = patient_id;
    c.name = name;

    heap[size] = c;
    int i = size++;

    /* Sift up */
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (higher_priority(&heap[i], &heap[parent])) {
            swap(&heap[i], &heap[parent]);
            i = parent;
        } else {
            break;
        }
    }
}

static PatientCase pop(void)
{
    PatientCase top = heap[0];
    heap[0] = heap[--size];
    int i = 0;

    /* Sift down */
    while (2 * i + 1 < size) {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int best = i;

        if (left < size && higher_priority(&heap[left], &heap[best])) {
            best = left;
        }
        if (right < size && higher_priority(&heap[right], &heap[best])) {
            best = right;
        }

        if (best == i) break;
        swap(&heap[i], &heap[best]);
        i = best;
    }
    return top;
}

int main(void)
{
    FILE *out = fopen("Task03/output/fair_heap_output.txt", "w");
    if (!out) {
        perror("Failed to open fair_heap_output.txt");
        return 1;
    }

    fprintf(out, "================================================================================\n");
    fprintf(out, "          FAIR TRIAGE PRIORITY QUEUE DEMO (FIFO TIE-BREAKING)\n");
    fprintf(out, "================================================================================\n\n");
    fprintf(out, "Arrival Sequence:\n");

    struct { int sev; int id; const char *name; } incoming[] = {
        {8, 201, "Alice  (Arrival 1, Sev 8)"},
        {5, 202, "Bob    (Arrival 2, Sev 5)"},
        {8, 203, "Charlie(Arrival 3, Sev 8 - same as Alice)"},
        {9, 204, "Diana  (Arrival 4, Sev 9 - critical)"},
        {8, 205, "Edward (Arrival 5, Sev 8 - same as Alice & Charlie)"},
        {5, 206, "Fiona  (Arrival 6, Sev 5 - same as Bob)"}
    };
    int count = sizeof(incoming) / sizeof(incoming[0]);

    for (int i = 0; i < count; i++) {
        push(incoming[i].sev, incoming[i].id, incoming[i].name);
        fprintf(out, "  [Arrived #%d] P%d : %s\n", i + 1, incoming[i].id, incoming[i].name);
    }

    fprintf(out, "\nTreatment Dispatch Order (Highest Severity first, FIFO on equal severity):\n");
    int order = 1;
    while (size > 0) {
        PatientCase c = pop();
        fprintf(out, "  Dispatch #%d: P%d [Severity %d, Seq #%d] -> %s\n",
                order++, c.patient_id, c.severity, c.seq_num, c.name);
    }

    fprintf(out, "\nProof of Fairness:\n");
    fprintf(out, "1. Severity 9: Diana (arrived #4) treated first immediately.\n");
    fprintf(out, "2. Severity 8: Alice (#1) -> Charlie (#3) -> Edward (#5) in STRICT arrival sequence!\n");
    fprintf(out, "3. Severity 5: Bob (#2) -> Fiona (#6) in STRICT arrival sequence!\n");

    fclose(out);
    printf("Fair heap demo complete. Output saved to Task03/output/fair_heap_output.txt\n");
    return 0;
}
