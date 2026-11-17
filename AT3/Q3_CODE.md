# Question 3: B-Tree

## btree.c

```c
/**
 * ============================================================================
 * Assignment 3 - CO3: Optimized Data Storage and Retrieval Using Tree Structures
 * Question 3: B-Tree (Hospital Patient Database on Disk)
 * File: btree.c
 * 
 * Standard: C11 (gcc -Wall -Wextra -std=c11 -g)
 * Author: Student
 * Description: B-Tree implementation with minimum degree t = 2 (Order 4).
 *              Supports proactive top-down splitting, insertion with full split
 *              logging, disk-read simulator tracking node accesses,
 *              search for smallest, largest, and missing keys, and theoretical
 *              5-million patient record disk I/O analysis (BST vs AVL vs B-Tree).
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#define T 2 /* Minimum degree: each node has [T-1, 2T-1] keys = [1, 3] keys */
#define MAX_KEYS (2 * T - 1) /* 3 keys */
#define MAX_CHILDREN (2 * T) /* 4 children */

typedef struct BTreeNode {
    int keys[MAX_KEYS];
    struct BTreeNode *children[MAX_CHILDREN];
    int num_keys;
    bool is_leaf;
} BTreeNode;

/* -------------------------------------------------------------------------
 * Allocation & Initialization
 * ------------------------------------------------------------------------- */
BTreeNode *create_btree_node(bool is_leaf) {
    BTreeNode *node = (BTreeNode *)malloc(sizeof(BTreeNode));
    if (!node) {
        fprintf(stderr, "Fatal: B-Tree node allocation failed\n");
        exit(EXIT_FAILURE);
    }
    node->num_keys = 0;
    node->is_leaf = is_leaf;
    for (int i = 0; i < MAX_CHILDREN; i++) {
        node->children[i] = NULL;
    }
    return node;
}

/* -------------------------------------------------------------------------
 * Node Splitting
 * Splits full child y of node x at index i.
 * ------------------------------------------------------------------------- */
void split_child(BTreeNode *x, int i, BTreeNode *y) {
    printf("    >>> [SPLIT EVENT] Node containing keys [");
    for (int k = 0; k < y->num_keys; k++) printf("%d%s", y->keys[k], (k < y->num_keys - 1) ? ", " : "");
    printf("] is FULL (capacity %d). Median key '%d' promotes to parent.\n", MAX_KEYS, y->keys[T - 1]);

    /* Create new node z which will store (T - 1) keys of y */
    BTreeNode *z = create_btree_node(y->is_leaf);
    z->num_keys = T - 1; /* 1 key */

    /* Copy last (T - 1) keys of y to z */
    for (int j = 0; j < T - 1; j++) {
        z->keys[j] = y->keys[j + T];
    }

    /* Copy last T children of y to z if not a leaf */
    if (!y->is_leaf) {
        for (int j = 0; j < T; j++) {
            z->children[j] = y->children[j + T];
            y->children[j + T] = NULL;
        }
    }

    y->num_keys = T - 1; /* Reduce key count of y */

    /* Shift children of x right to create space for z */
    for (int j = x->num_keys; j >= i + 1; j--) {
        x->children[j + 1] = x->children[j];
    }
    x->children[i + 1] = z;

    /* Shift keys of x right to create space for median key of y */
    for (int j = x->num_keys - 1; j >= i; j--) {
        x->keys[j + 1] = x->keys[j];
    }

    /* Insert median key of y into x */
    x->keys[i] = y->keys[T - 1];
    x->num_keys++;
}

/* -------------------------------------------------------------------------
 * Insertion (Proactive Top-Down Non-Full Insertion)
 * ------------------------------------------------------------------------- */
void insert_non_full(BTreeNode *x, int key) {
    int i = x->num_keys - 1;

    if (x->is_leaf) {
        /* Insert key into sorted position in leaf */
        while (i >= 0 && x->keys[i] > key) {
            x->keys[i + 1] = x->keys[i];
            i--;
        }
        x->keys[i + 1] = key;
        x->num_keys++;
    } else {
        /* Locate child that will receive the key */
        while (i >= 0 && x->keys[i] > key) {
            i--;
        }
        i++;

        /* If child is full, split it proactively */
        if (x->children[i]->num_keys == MAX_KEYS) {
            split_child(x, i, x->children[i]);
            if (x->keys[i] < key) {
                i++;
            }
        }
        insert_non_full(x->children[i], key);
    }
}

BTreeNode *btree_insert(BTreeNode *root, int key) {
    if (!root) {
        root = create_btree_node(true);
        root->keys[0] = key;
        root->num_keys = 1;
        return root;
    }

    /* If root is full, tree grows in height */
    if (root->num_keys == MAX_KEYS) {
        printf("    >>> [ROOT SPLIT] Root was full! Allocating new root; tree height increases by 1.\n");
        BTreeNode *s = create_btree_node(false);
        s->children[0] = root;
        split_child(s, 0, root);

        int i = 0;
        if (s->keys[0] < key) i++;
        insert_non_full(s->children[i], key);
        return s;
    } else {
        insert_non_full(root, key);
        return root;
    }
}

/* -------------------------------------------------------------------------
 * Search with Disk Read Counter
 * ------------------------------------------------------------------------- */
bool btree_search(const BTreeNode *x, int key, int *disk_reads) {
    if (!x) return false;
    (*disk_reads)++; /* Each node access represents 1 Disk Page Read */

    int i = 0;
    while (i < x->num_keys && key > x->keys[i]) {
        i++;
    }

    if (i < x->num_keys && key == x->keys[i]) {
        return true; /* Key found in current disk block */
    }

    if (x->is_leaf) {
        return false; /* Reached leaf, key absent */
    }

    return btree_search(x->children[i], key, disk_reads);
}

/* -------------------------------------------------------------------------
 * Display & Print Utilities
 * ------------------------------------------------------------------------- */
void print_btree(const BTreeNode *node, int level) {
    if (!node) return;
    for (int i = 0; i < level; i++) printf("    ");
    printf("Level %d %s [", level, node->is_leaf ? "(Leaf)" : "(Internal)");
    for (int i = 0; i < node->num_keys; i++) {
        printf("%d%s", node->keys[i], (i < node->num_keys - 1) ? " " : "");
    }
    printf("]\n");

    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            print_btree(node->children[i], level + 1);
        }
    }
}

void free_btree(BTreeNode *node) {
    if (!node) return;
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; i++) {
            free_btree(node->children[i]);
        }
    }
    free(node);
}

/* -------------------------------------------------------------------------
 * Disk-Read Simulation for 5 Million Patient Records
 * ------------------------------------------------------------------------- */
void run_disk_simulation(void) {
    const double N_RECORDS = 5000000.0;
    const double DISK_SEEK_MS = 10.0;
    const int ORDER = 100; /* Order M = 100 -> min degree t = 50 */
    const int T_VAL = 50;

    printf("\n=================================================================\n");
    printf("   DISK-READ SIMULATOR: 5 MILLION PATIENT RECORDS (10ms I/O)    \n");
    printf("=================================================================\n\n");

    /* 3. B-Tree (Order 100, t = 50) Analysis:
     * Minimum height (full nodes, M children): h_min = ceil(log_M(N))
     * Maximum height (minimally filled nodes, t children): h_max <= 1 + log_t((N+1)/2)
     */
    double btree_h_min = ceil(log(N_RECORDS) / log(ORDER));
    double btree_h_max = ceil(log((N_RECORDS + 1) / 2.0) / log(T_VAL));

    printf("+-------------------------------+---------------+---------------+-------------------+\n");
    printf("| Tree Structure                | Min Node Reads| Max Node Reads| Search Time @10ms |\n");
    printf("+-------------------------------+---------------+---------------+-------------------+\n");
    printf("| Plain BST (Worst Case)        | 5,000,000     | 5,000,000     | 50,000.00 seconds |\n");
    printf("| Plain BST (Average Random)    | 22            | 32            | 220 - 320 ms      |\n");
    printf("| AVL Tree (Guaranteed O(log N))| 23            | 33            | 230 - 330 ms      |\n");
    printf("| B-Tree (Order 100, t = 50)    | %-13.0f | %-13.0f | %.0f - %.0f ms        |\n",
           btree_h_min, btree_h_max, btree_h_min * DISK_SEEK_MS, btree_h_max * DISK_SEEK_MS);
    printf("+-------------------------------+---------------+---------------+-------------------+\n\n");

    printf("Analysis of Disk I/O Bottleneck:\n");
    printf("1. An in-memory pointer dereference takes ~1-10 nanoseconds.\n");
    printf("2. A physical spinning disk / networked block seek takes ~10 milliseconds (1,000,000x slower!).\n");
    printf("3. While AVL requires ~23-33 seeks (taking ~300 ms), a B-tree needs only 4 disk block reads (40 ms).\n");
    printf("4. With the top 2 levels cached in RAM, the B-Tree completes in just 1-2 disk reads (~10-20 ms)!\n\n");
}

/* -------------------------------------------------------------------------
 * Main Program
 * ------------------------------------------------------------------------- */
int main(void) {
    printf("=================================================================\n");
    printf("         QUESTION 3: B-TREE (HOSPITAL PATIENT DATABASE)          \n");
    printf("=================================================================\n\n");

    /* ---------------------------------------------------------------------
     * Part A: Inserting 10, 20, 5, 6, 12, 30, 7, 17 into B-tree (t = 2)
     * --------------------------------------------------------------------- */
    int insert_keys[] = {10, 20, 5, 6, 12, 30, 7, 17};
    int n = sizeof(insert_keys) / sizeof(insert_keys[0]);
    BTreeNode *root = NULL;

    printf("--- PART A: STEP-BY-STEP INSERTIONS WITH SPLIT LOGGING (t=2, Order=4) ---\n\n");
    for (int i = 0; i < n; i++) {
        printf(">> [STEP %d] Inserting Key: %d\n", i + 1, insert_keys[i]);
        root = btree_insert(root, insert_keys[i]);
        printf("Current B-Tree Structure:\n");
        print_btree(root, 0);
        printf("-----------------------------------------------------------------\n");
    }

    /* ---------------------------------------------------------------------
     * Part B: Search Tests (Smallest, Largest, Missing)
     * --------------------------------------------------------------------- */
    printf("\n--- PART B: SEARCH TESTS WITH DISK-READ TRACKING ---\n");
    int test_queries[] = {5, 30, 15, 99};
    const char *query_labels[] = {
        "Smallest Key (5)",
        "Largest Key (30)",
        "Missing Key (15, middle gap)",
        "Missing Key (99, out-of-range)"
    };

    for (int i = 0; i < 4; i++) {
        int disk_reads = 0;
        bool found = btree_search(root, test_queries[i], &disk_reads);
        printf("Search for %-32s -> Status: %-10s | Disk Reads (Node Visits): %d\n",
               query_labels[i], found ? "FOUND" : "NOT FOUND", disk_reads);
    }

    /* ---------------------------------------------------------------------
     * Part C: 5-Million Record Disk Simulation
     * --------------------------------------------------------------------- */
    run_disk_simulation();

    /* Clean up memory */
    free_btree(root);
    printf("B-Tree memory successfully deallocated.\n");
    return 0;
}
```

