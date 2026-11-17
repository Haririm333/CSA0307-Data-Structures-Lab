/**
 * ============================================================================
 * Assignment 3 - CO3: Optimized Data Storage and Retrieval Using Tree Structures
 * Question 5: Splay Tree (E-Commerce Frequently Viewed Products Cache)
 * File: splay.c
 * 
 * Standard: C11 (gcc -Wall -Wextra -std=c11 -g)
 * Author: Student
 * Description: Splay Tree implementation with Zig, Zig-Zig, and Zig-Zag
 *              rotations, step-by-step splay demonstration for keys 3 and 9,
 *              10,000 access simulation comparing 80/5 skewed (Pareto) workload
 *              versus uniform workload against an AVL tree, worst-single-access
 *              tracking, and first 10 access root logging.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

typedef struct SplayNode {
    int key;                    /* Product ID */
    struct SplayNode *left;
    struct SplayNode *right;
} SplayNode;

/* -------------------------------------------------------------------------
 * Node Allocation
 * ------------------------------------------------------------------------- */
SplayNode *create_splay_node(int key) {
    SplayNode *n = (SplayNode *)malloc(sizeof(SplayNode));
    if (!n) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }
    n->key = key;
    n->left = n->right = NULL;
    return n;
}

/* -------------------------------------------------------------------------
 * Rotations
 * ------------------------------------------------------------------------- */
SplayNode *splay_rotate_right(SplayNode *y) {
    SplayNode *x = y->left;
    y->left = x->right;
    x->right = y;
    return x;
}

SplayNode *splay_rotate_left(SplayNode *x) {
    SplayNode *y = x->right;
    x->right = y->left;
    y->left = x;
    return y;
}

/* -------------------------------------------------------------------------
 * Top-Down Splaying (Sleator & Tarjan)
 * Brings key to root if present, or brings the last accessed node to root.
 * Accurately performs:
 *  - Zig (single rotation)
 *  - Zig-Zig (same direction: rotate grandparent then parent)
 *  - Zig-Zag (opposite direction: rotate parent then grandparent)
 * ------------------------------------------------------------------------- */
SplayNode *splay(SplayNode *root, int key, int *visits) {
    if (!root) return NULL;

    SplayNode header;
    header.left = header.right = NULL;
    SplayNode *left_tree_max = &header;
    SplayNode *right_tree_min = &header;

    SplayNode *t = root;
    while (1) {
        if (visits) (*visits)++;

        if (key < t->key) {
            if (!t->left) break;
            if (key < t->left->key) {
                /* Zig-Zig (Left-Left): rotate right */
                t = splay_rotate_right(t);
                if (visits) (*visits)++;
                if (!t->left) break;
            }
            /* Link Right: attach t to right_tree_min */
            right_tree_min->left = t;
            right_tree_min = t;
            t = t->left;
        } else if (key > t->key) {
            if (!t->right) break;
            if (key > t->right->key) {
                /* Zig-Zig (Right-Right): rotate left */
                t = splay_rotate_left(t);
                if (visits) (*visits)++;
                if (!t->right) break;
            }
            /* Link Left: attach t to left_tree_max */
            left_tree_max->right = t;
            left_tree_max = t;
            t = t->right;
        } else {
            break; /* Key matched */
        }
    }

    /* Reassemble left, middle, and right trees */
    left_tree_max->right = t->left;
    right_tree_min->left = t->right;
    t->left = header.right;
    t->right = header.left;

    return t;
}

/* Splay Insert: Splays at key, then attaches new node as root */
SplayNode *splay_insert(SplayNode *root, int key) {
    if (!root) return create_splay_node(key);

    int visits = 0;
    root = splay(root, key, &visits);
    if (root->key == key) return root; /* Duplicate */

    SplayNode *new_node = create_splay_node(key);
    if (key < root->key) {
        new_node->right = root;
        new_node->left = root->left;
        root->left = NULL;
    } else {
        new_node->left = root;
        new_node->right = root->right;
        root->right = NULL;
    }
    return new_node;
}

/* Splay Search: Brings searched item to root */
SplayNode *splay_search(SplayNode *root, int key, int *visits, bool *found) {
    *visits = 0;
    if (!root) {
        *found = false;
        return NULL;
    }
    root = splay(root, key, visits);
    *found = (root->key == key);
    return root;
}

/* -------------------------------------------------------------------------
 * Display & Visualizer
 * ------------------------------------------------------------------------- */
static void print_ascii_helper(const SplayNode *root, int space) {
    const int COUNT = 6;
    if (!root) return;
    space += COUNT;
    print_ascii_helper(root->right, space);
    printf("\n");
    for (int i = COUNT; i < space; i++) printf(" ");
    printf("[%d]\n", root->key);
    print_ascii_helper(root->left, space);
}

void print_splay_ascii(const SplayNode *root) {
    if (!root) {
        printf("(empty tree)\n");
        return;
    }
    print_ascii_helper(root, 0);
    printf("--------------------------------------------------\n");
}

void free_splay(SplayNode *root) {
    if (!root) return;
    free_splay(root->left);
    free_splay(root->right);
    free(root);
}

/* -------------------------------------------------------------------------
 * Baseline AVL Tree for Comparative Experiment
 * ------------------------------------------------------------------------- */
typedef struct AVLNodeSimple {
    int key;
    int height;
    struct AVLNodeSimple *left, *right;
} AVLNodeSimple;

static int avl_ht(const AVLNodeSimple *n) { return n ? n->height : 0; }
static int avl_max(int a, int b) { return a > b ? a : b; }
static void avl_fix(AVLNodeSimple *n) { n->height = 1 + avl_max(avl_ht(n->left), avl_ht(n->right)); }

static AVLNodeSimple *avl_rot_r(AVLNodeSimple *y) {
    AVLNodeSimple *x = y->left;
    y->left = x->right;
    x->right = y;
    avl_fix(y);
    avl_fix(x);
    return x;
}

static AVLNodeSimple *avl_rot_l(AVLNodeSimple *x) {
    AVLNodeSimple *y = x->right;
    x->right = y->left;
    y->left = x;
    avl_fix(x);
    avl_fix(y);
    return y;
}

static AVLNodeSimple *avl_insert_simple(AVLNodeSimple *n, int key) {
    if (!n) {
        AVLNodeSimple *node = (AVLNodeSimple *)malloc(sizeof(AVLNodeSimple));
        node->key = key; node->height = 1; node->left = node->right = NULL;
        return node;
    }
    if (key < n->key) n->left = avl_insert_simple(n->left, key);
    else if (key > n->key) n->right = avl_insert_simple(n->right, key);
    else return n;
    avl_fix(n);
    int bal = avl_ht(n->left) - avl_ht(n->right);
    if (bal > 1 && key < n->left->key) return avl_rot_r(n);
    if (bal < -1 && key > n->right->key) return avl_rot_l(n);
    if (bal > 1) { n->left = avl_rot_l(n->left); return avl_rot_r(n); }
    if (bal < -1) { n->right = avl_rot_r(n->right); return avl_rot_l(n); }
    return n;
}

static bool avl_search_count(const AVLNodeSimple *root, int key, int *visits) {
    *visits = 0;
    const AVLNodeSimple *curr = root;
    while (curr) {
        (*visits)++;
        if (key == curr->key) return true;
        curr = (key < curr->key) ? curr->left : curr->right;
    }
    return false;
}

static void free_avl_simple(AVLNodeSimple *n) {
    if (!n) return;
    free_avl_simple(n->left);
    free_avl_simple(n->right);
    free(n);
}

/* -------------------------------------------------------------------------
 * Main Benchmark Driver
 * ------------------------------------------------------------------------- */
int main(void) {
    printf("=================================================================\n");
    printf("     QUESTION 5: SPLAY TREE (PRODUCT CACHE - 80/5 WORKLOAD)      \n");
    printf("=================================================================\n\n");

    /* ---------------------------------------------------------------------
     * Part A: Step-by-Step Splay of 3 then 9
     * --------------------------------------------------------------------- */
    printf("--- PART A: SPLAYING KEY 3 THEN KEY 9 IN SAMPLE TREE ---\n");
    SplayNode *demo_tree = NULL;
    int initial_keys[] = {5, 2, 8, 1, 4, 7, 10, 3, 9};
    for (int i = 0; i < 9; i++) {
        demo_tree = splay_insert(demo_tree, initial_keys[i]);
    }
    printf("Initial Sample Tree Built with {5, 2, 8, 1, 4, 7, 10, 3, 9}:\n");
    print_splay_ascii(demo_tree);

    int visits = 0;
    bool found = false;

    /* Step 1: Access key 3 */
    printf(">> Accessing Product ID 3 (Splay brings 3 to root):\n");
    demo_tree = splay_search(demo_tree, 3, &visits, &found);
    printf("   Status: %s | Nodes visited: %d\n", found ? "FOUND" : "NOT FOUND", visits);
    printf("   Tree state with root = %d:\n", demo_tree->key);
    print_splay_ascii(demo_tree);

    /* Step 2: Access key 9 */
    printf(">> Accessing Product ID 9 (Splay brings 9 to root):\n");
    demo_tree = splay_search(demo_tree, 9, &visits, &found);
    printf("   Status: %s | Nodes visited: %d\n", found ? "FOUND" : "NOT FOUND", visits);
    printf("   Tree state with root = %d:\n", demo_tree->key);
    print_splay_ascii(demo_tree);
    free_splay(demo_tree);

    /* ---------------------------------------------------------------------
     * Part B: 10,000 Access Simulation (80/5 Skewed vs Uniform Workload)
     * 1,000 Products (Keys 1 to 1,000)
     * 80% of accesses hit 5% of products (Keys 1 to 50)
     * --------------------------------------------------------------------- */
    printf("\n--- PART B: 10,000 ACCESSES EXPERIMENT (SPLAY VS AVL) ---\n");
    const int NUM_PRODUCTS = 1000;
    const int NUM_ACCESSES = 10000;

    SplayNode *splay_tree = NULL;
    AVLNodeSimple *avl_tree = NULL;

    for (int i = 1; i <= NUM_PRODUCTS; i++) {
        splay_tree = splay_insert(splay_tree, i);
        avl_tree = avl_insert_simple(avl_tree, i);
    }

    /* 1. Skewed Workload (80% hits 5% popular products) */
    printf("1. Running 80/5 Skewed Workload (10,000 accesses)...\n");
    srand(777);
    long long splay_skewed_visits = 0;
    long long avl_skewed_visits = 0;
    int splay_worst_skewed = 0;
    int avl_worst_skewed = 0;
    int first_ten_roots[10];

    for (int i = 0; i < NUM_ACCESSES; i++) {
        int key;
        if ((rand() % 100) < 80) {
            /* 80% probability: pick from hot 5% (keys 1 to 50) */
            key = 1 + (rand() % 50);
        } else {
            /* 20% probability: pick from remaining 95% (keys 51 to 1000) */
            key = 51 + (rand() % 950);
        }

        int s_vis = 0, a_vis = 0;
        bool f1 = false;
        splay_tree = splay_search(splay_tree, key, &s_vis, &f1);
        avl_search_count(avl_tree, key, &a_vis);

        splay_skewed_visits += s_vis;
        avl_skewed_visits += a_vis;

        if (s_vis > splay_worst_skewed) splay_worst_skewed = s_vis;
        if (a_vis > avl_worst_skewed) avl_worst_skewed = a_vis;

        if (i < 10) {
            first_ten_roots[i] = splay_tree->key;
        }
    }

    /* 2. Uniform Workload */
    printf("2. Running Uniform Workload (10,000 accesses)...\n");
    srand(888);
    long long splay_uniform_visits = 0;
    long long avl_uniform_visits = 0;
    int splay_worst_uniform = 0;
    int avl_worst_uniform = 0;

    for (int i = 0; i < NUM_ACCESSES; i++) {
        int key = 1 + (rand() % NUM_PRODUCTS);
        int s_vis = 0, a_vis = 0;
        bool f1 = false;
        splay_tree = splay_search(splay_tree, key, &s_vis, &f1);
        avl_search_count(avl_tree, key, &a_vis);

        splay_uniform_visits += s_vis;
        avl_uniform_visits += a_vis;

        if (s_vis > splay_worst_uniform) splay_worst_uniform = s_vis;
        if (a_vis > avl_worst_uniform) avl_worst_uniform = a_vis;
    }

    /* 3. Sequential Access Test (1 to 1000 in sorted order) */
    printf("3. Testing Sequential Sorted Access (Keys 1 to 1000)...\n");
    long long splay_seq_visits = 0;
    for (int i = 1; i <= NUM_PRODUCTS; i++) {
        int s_vis = 0;
        bool f1 = false;
        splay_tree = splay_search(splay_tree, i, &s_vis, &f1);
        splay_seq_visits += s_vis;
    }

    /* Results Table */
    printf("\n+---------------------------------------+-------------------+-------------------+\n");
    printf("| Access Metric (10,000 Operations)     | Splay Tree        | AVL Tree          |\n");
    printf("+---------------------------------------+-------------------+-------------------+\n");
    printf("| Skewed (80/5) Avg Nodes Visited       | %-17.2f | %-17.2f |\n",
           (double)splay_skewed_visits / NUM_ACCESSES, (double)avl_skewed_visits / NUM_ACCESSES);
    printf("| Skewed Worst Single Access            | %-17d | %-17d |\n",
           splay_worst_skewed, avl_worst_skewed);
    printf("| Uniform Avg Nodes Visited             | %-17.2f | %-17.2f |\n",
           (double)splay_uniform_visits / NUM_ACCESSES, (double)avl_uniform_visits / NUM_ACCESSES);
    printf("| Uniform Worst Single Access           | %-17d | %-17d |\n",
           splay_worst_uniform, avl_worst_uniform);
    printf("| Sequential Access Avg Nodes (1..1000) | %-17.2f | 9.97 (log2 N)     |\n",
           (double)splay_seq_visits / NUM_PRODUCTS);
    printf("+---------------------------------------+-------------------+-------------------+\n\n");

    /* Record of First 10 Access Roots */
    printf("Roots after each of the first 10 skewed accesses:\n");
    for (int i = 0; i < 10; i++) {
        printf("  Access %2d: Root Key = %d (accessed item immediately promoted to root!)\n",
               i + 1, first_ten_roots[i]);
    }
    printf("\n");

    /* Cleanup */
    free_splay(splay_tree);
    free_avl_simple(avl_tree);

    printf("All splay and benchmark nodes freed. Zero leaks.\n");
    return 0;
}
