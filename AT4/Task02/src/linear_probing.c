#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BUCKETS 11

typedef enum { EMPTY = 0, OCCUPIED, DELETED } SlotStatus;

typedef struct {
    char id[12];
    char name[20];
    SlotStatus status;
} TableEntry;

static TableEntry table[BUCKETS];

static unsigned hash(const char *s)
{
    unsigned h = 0;
    while (*s) h = h * 31 + (unsigned char)*s++;
    return h % BUCKETS;
}

static bool put(const char *id, const char *name, int *probes)
{
    *probes = 0;
    unsigned h = hash(id);
    int first_deleted = -1;

    for (int step = 0; step < BUCKETS; step++) {
        (*probes)++;
        unsigned idx = (h + step) % BUCKETS;

        if (table[idx].status == OCCUPIED) {
            if (strcmp(table[idx].id, id) == 0) {
                /* Update existing */
                strncpy(table[idx].name, name, sizeof(table[idx].name) - 1);
                table[idx].name[sizeof(table[idx].name) - 1] = '\0';
                return true;
            }
        } else if (table[idx].status == DELETED) {
            if (first_deleted == -1) first_deleted = idx;
        } else { /* EMPTY */
            int target = (first_deleted != -1) ? first_deleted : (int)idx;
            strncpy(table[target].id, id, sizeof(table[target].id) - 1);
            table[target].id[sizeof(table[target].id) - 1] = '\0';
            strncpy(table[target].name, name, sizeof(table[target].name) - 1);
            table[target].name[sizeof(table[target].name) - 1] = '\0';
            table[target].status = OCCUPIED;
            return true;
        }
    }

    if (first_deleted != -1) {
        strncpy(table[first_deleted].id, id, sizeof(table[first_deleted].id) - 1);
        table[first_deleted].id[sizeof(table[first_deleted].id) - 1] = '\0';
        strncpy(table[first_deleted].name, name, sizeof(table[first_deleted].name) - 1);
        table[first_deleted].name[sizeof(table[first_deleted].name) - 1] = '\0';
        table[first_deleted].status = OCCUPIED;
        return true;
    }

    return false; /* Table full */
}

static const char *get(const char *id, int *probes)
{
    *probes = 0;
    unsigned h = hash(id);

    for (int step = 0; step < BUCKETS; step++) {
        (*probes)++;
        unsigned idx = (h + step) % BUCKETS;

        if (table[idx].status == EMPTY) {
            return NULL; /* Stop on empty slot */
        }
        if (table[idx].status == OCCUPIED && strcmp(table[idx].id, id) == 0) {
            return table[idx].name;
        }
        /* Skip DELETED and keep probing */
    }
    return NULL;
}

int main(void)
{
    const char *ids[] = {"CSE1043", "ECE2210", "MEC3307",
                         "CSE1044", "BME4120", "CIV5009"};
    const char *names[] = {"Kavya", "Arjun", "Fathima", "Rahul", "Deepa", "Manoj"};

    printf("=== Linear Probing Demonstration (M = %d) ===\n", BUCKETS);
    for (int i = 0; i < 6; i++) {
        int p = 0;
        put(ids[i], names[i], &p);
        printf("Inserted %s (%s) with %d probe(s)\n", ids[i], names[i], p);
    }

    printf("\nTable Slots:\n");
    for (int i = 0; i < BUCKETS; i++) {
        if (table[i].status == OCCUPIED) {
            printf("Slot %2d: [%s : %s]\n", i, table[i].id, table[i].name);
        } else {
            printf("Slot %2d: [EMPTY]\n", i);
        }
    }

    int p;
    const char *who = get("CSE1044", &p);
    printf("\nLookup 'CSE1044': %s (%d probes)\n", who ? who : "not found", p);

    who = get("CSE9999", &p);
    printf("Lookup 'CSE9999': %s (%d probes)\n", who ? who : "not found", p);

    return 0;
}
