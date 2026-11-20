#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Node {
    char id[12];
    char name[24];
    struct Node *next;
} Node;

typedef struct {
    int capacity;
    int count;
    double max_load_factor;
    Node **buckets;
} HashTable;

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

static unsigned hash_str(const char *s, int m)
{
    unsigned h = 0;
    while (*s) h = h * 31 + (unsigned char)*s++;
    return h % (unsigned)m;
}

static HashTable *create_table(int init_cap, double max_lf)
{
    HashTable *ht = malloc(sizeof(HashTable));
    ht->capacity = init_cap;
    ht->count = 0;
    ht->max_load_factor = max_lf;
    ht->buckets = calloc(init_cap, sizeof(Node *));
    return ht;
}

static void rehash(HashTable *ht, FILE *out)
{
    int old_cap = ht->capacity;
    int new_cap = next_prime(old_cap * 2);
    fprintf(out, "\n>>> [REHASH TRIGGERED] Load factor %.2f exceeds threshold %.2f!\n",
            (double)ht->count / old_cap, ht->max_load_factor);
    fprintf(out, "    Expanding table: capacity %d -> %d (next prime >= 2x)\n", old_cap, new_cap);

    Node **new_buckets = calloc(new_cap, sizeof(Node *));
    for (int i = 0; i < old_cap; i++) {
        Node *curr = ht->buckets[i];
        while (curr) {
            Node *next = curr->next;
            unsigned new_idx = hash_str(curr->id, new_cap);
            curr->next = new_buckets[new_idx];
            new_buckets[new_idx] = curr;
            fprintf(out, "    Rehashed %-8s : bucket %2d -> bucket %2d\n", curr->id, i, new_idx);
            curr = next;
        }
    }

    free(ht->buckets);
    ht->buckets = new_buckets;
    ht->capacity = new_cap;
    fprintf(out, ">>> [REHASH COMPLETE] New capacity = %d, count = %d, new load factor = %.2f\n\n",
            ht->capacity, ht->count, (double)ht->count / ht->capacity);
}

static void insert(HashTable *ht, const char *id, const char *name, FILE *out)
{
    double current_lf = (double)(ht->count + 1) / ht->capacity;
    if (current_lf > ht->max_load_factor) {
        rehash(ht, out);
    }

    unsigned b = hash_str(id, ht->capacity);
    Node *n = malloc(sizeof(Node));
    strncpy(n->id, id, sizeof(n->id) - 1);
    n->id[sizeof(n->id) - 1] = '\0';
    strncpy(n->name, name, sizeof(n->name) - 1);
    n->name[sizeof(n->name) - 1] = '\0';
    n->next = ht->buckets[b];
    ht->buckets[b] = n;
    ht->count++;

    fprintf(out, "Inserted %-8s (%-10s) -> bucket %2d [Total: %2d/%2d, Load: %.2f]\n",
            id, name, b, ht->count, ht->capacity, (double)ht->count / ht->capacity);
}

static void free_table(HashTable *ht)
{
    for (int i = 0; i < ht->capacity; i++) {
        Node *curr = ht->buckets[i];
        while (curr) {
            Node *tmp = curr->next;
            free(curr);
            curr = tmp;
        }
    }
    free(ht->buckets);
    free(ht);
}

int main(void)
{
    FILE *log = fopen("Task02/output/rehash_log.txt", "w");
    if (!log) {
        perror("Failed to open rehash_log.txt");
        return 1;
    }

    fprintf(log, "================================================================================\n");
    fprintf(log, "           DYNAMIC REHASHING DEMONSTRATION (Separate Chaining)\n");
    fprintf(log, "================================================================================\n");
    fprintf(log, "Initial capacity: 7 (Prime), Max Load Factor Threshold: 0.75\n\n");

    HashTable *ht = create_table(7, 0.75);

    struct { const char *id; const char *name; } members[] = {
        {"CSE1043", "Kavya"},
        {"ECE2210", "Arjun"},
        {"MEC3307", "Fathima"},
        {"CSE1044", "Rahul"},
        {"BME4120", "Deepa"},
        {"CIV5009", "Manoj"},
        {"CSE2011", "Sneha"},
        {"ECE3022", "Vikram"},
        {"IT4033",  "Ananya"},
        {"EEE5044", "Rohan"},
        {"BIO6055", "Pooja"},
        {"AER7066", "Aditya"}
    };
    int total_members = sizeof(members) / sizeof(members[0]);

    for (int i = 0; i < total_members; i++) {
        insert(ht, members[i].id, members[i].name, log);
    }

    fprintf(log, "\nFinal Table State:\n");
    for (int i = 0; i < ht->capacity; i++) {
        fprintf(log, "Bucket %2d: ", i);
        Node *curr = ht->buckets[i];
        if (!curr) {
            fprintf(log, "[EMPTY]\n");
        } else {
            while (curr) {
                fprintf(log, "[%s: %s] -> ", curr->id, curr->name);
                curr = curr->next;
            }
            fprintf(log, "NULL\n");
        }
    }

    free_table(ht);
    fclose(log);
    printf("Rehashing demonstration complete. Log saved to Task02/output/rehash_log.txt\n");
    return 0;
}
