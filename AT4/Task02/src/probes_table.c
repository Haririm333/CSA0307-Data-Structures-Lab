#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define N_KEYS 40000

/* Multiplicative integer hash function for uniform distribution */
static inline unsigned hash_int(unsigned x, unsigned m)
{
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x % m;
}

/* Chaining structures */
typedef struct CNode {
    int key;
    struct CNode *next;
} CNode;

typedef struct {
    int size;
    CNode **buckets;
} ChainingTable;

static ChainingTable *create_chaining(int size)
{
    ChainingTable *t = malloc(sizeof(ChainingTable));
    t->size = size;
    t->buckets = calloc(size, sizeof(CNode *));
    return t;
}

static void insert_chaining(ChainingTable *t, int key)
{
    unsigned h = hash_int((unsigned)key, t->size);
    CNode *node = malloc(sizeof(CNode));
    node->key = key;
    node->next = t->buckets[h];
    t->buckets[h] = node;
}

static int search_chaining(ChainingTable *t, int key)
{
    unsigned h = hash_int((unsigned)key, t->size);
    int probes = 0;
    for (CNode *curr = t->buckets[h]; curr; curr = curr->next) {
        probes++;
        if (curr->key == key) return probes;
    }
    return probes; /* chain length checked */
}

static void free_chaining(ChainingTable *t)
{
    for (int i = 0; i < t->size; i++) {
        CNode *curr = t->buckets[i];
        while (curr) {
            CNode *tmp = curr->next;
            free(curr);
            curr = tmp;
        }
    }
    free(t->buckets);
    free(t);
}

/* Linear probing structures */
typedef struct {
    int size;
    int *table;
    bool *occupied;
} LPTable;

static LPTable *create_lp(int size)
{
    LPTable *t = malloc(sizeof(LPTable));
    t->size = size;
    t->table = malloc(size * sizeof(int));
    t->occupied = calloc(size, sizeof(bool));
    return t;
}

static void insert_lp(LPTable *t, int key)
{
    unsigned h = hash_int((unsigned)key, t->size);
    while (t->occupied[h]) {
        h = (h + 1) % t->size;
    }
    t->table[h] = key;
    t->occupied[h] = true;
}

static int search_lp(LPTable *t, int key, bool *found)
{
    unsigned h = hash_int((unsigned)key, t->size);
    int probes = 0;
    *found = false;
    for (int step = 0; step < t->size; step++) {
        probes++;
        unsigned idx = (h + step) % t->size;
        if (!t->occupied[idx]) {
            return probes;
        }
        if (t->table[idx] == key) {
            *found = true;
            return probes;
        }
    }
    return probes;
}

static void free_lp(LPTable *t)
{
    free(t->table);
    free(t->occupied);
    free(t);
}

int main(void)
{
    double alphas[] = {0.50, 0.75, 0.90};
    int n_alphas = 3;

    int *insert_keys = malloc(N_KEYS * sizeof(int));
    int *absent_keys = malloc(N_KEYS * sizeof(int));

    srand(1337);
    for (int i = 0; i < N_KEYS; i++) {
        insert_keys[i] = rand() * 100000 + i;
        absent_keys[i] = -(rand() * 100000 + i + 1);
    }

    FILE *csv = fopen("Task02/output/probes_table.csv", "w");
    if (!csv) {
        perror("Failed to open Task02/output/probes_table.csv");
        free(insert_keys);
        free(absent_keys);
        return 1;
    }

    fprintf(csv, "Alpha,Method,Theoretical_Succ,Empirical_Succ,Theoretical_Unsucc,Empirical_Unsucc\n");

    printf("=========================================================================================================\n");
    printf("                  THEORETICAL VS EMPIRICAL PROBES ANALYSIS (N = %d keys)\n", N_KEYS);
    printf("=========================================================================================================\n");
    printf("%-6s | %-16s | %-18s | %-18s | %-18s | %-18s\n",
           "Alpha", "Method", "Theory (Succ)", "Empirical (Succ)", "Theory (Unsucc)", "Empirical (Unsucc)");
    printf("---------------------------------------------------------------------------------------------------------\n");

    for (int a = 0; a < n_alphas; a++) {
        double alpha = alphas[a];
        int m = (int)round((double)N_KEYS / alpha);

        /* 1. Chaining */
        double theory_chain_succ = 1.0 + alpha / 2.0;
        double theory_chain_unsucc = alpha;

        ChainingTable *ct = create_chaining(m);
        for (int i = 0; i < N_KEYS; i++) {
            insert_chaining(ct, insert_keys[i]);
        }

        long long total_probes_succ = 0;
        for (int i = 0; i < N_KEYS; i++) {
            total_probes_succ += search_chaining(ct, insert_keys[i]);
        }
        double emp_chain_succ = (double)total_probes_succ / N_KEYS;

        long long total_probes_unsucc = 0;
        for (int i = 0; i < N_KEYS; i++) {
            total_probes_unsucc += search_chaining(ct, absent_keys[i]);
        }
        double emp_chain_unsucc = (double)total_probes_unsucc / N_KEYS;
        free_chaining(ct);

        /* 2. Linear Probing */
        double theory_lp_succ = 0.5 * (1.0 + 1.0 / (1.0 - alpha));
        double theory_lp_unsucc = 0.5 * (1.0 + 1.0 / ((1.0 - alpha) * (1.0 - alpha)));

        LPTable *lt = create_lp(m);
        for (int i = 0; i < N_KEYS; i++) {
            insert_lp(lt, insert_keys[i]);
        }

        total_probes_succ = 0;
        for (int i = 0; i < N_KEYS; i++) {
            bool found = false;
            total_probes_succ += search_lp(lt, insert_keys[i], &found);
        }
        double emp_lp_succ = (double)total_probes_succ / N_KEYS;

        total_probes_unsucc = 0;
        for (int i = 0; i < N_KEYS; i++) {
            bool found = false;
            total_probes_unsucc += search_lp(lt, absent_keys[i], &found);
        }
        double emp_lp_unsucc = (double)total_probes_unsucc / N_KEYS;
        free_lp(lt);

        printf("%-6.2f | %-16s | %18.2f | %18.2f | %18.2f | %18.2f\n",
               alpha, "Chaining", theory_chain_succ, emp_chain_succ, theory_chain_unsucc, emp_chain_unsucc);
        printf("%-6.2f | %-16s | %18.2f | %18.2f | %18.2f | %18.2f\n",
               alpha, "Linear Probing", theory_lp_succ, emp_lp_succ, theory_lp_unsucc, emp_lp_unsucc);
        printf("---------------------------------------------------------------------------------------------------------\n");

        fprintf(csv, "%.2f,Chaining,%.4f,%.4f,%.4f,%.4f\n",
                alpha, theory_chain_succ, emp_chain_succ, theory_chain_unsucc, emp_chain_unsucc);
        fprintf(csv, "%.2f,LinearProbing,%.4f,%.4f,%.4f,%.4f\n",
                alpha, theory_lp_succ, emp_lp_succ, theory_lp_unsucc, emp_lp_unsucc);
    }

    fclose(csv);
    free(insert_keys);
    free(absent_keys);
    printf("CSV saved to Task02/output/probes_table.csv\n");
    return 0;
}
