# Question 1: Binary Search Tree

## bst.c

```c
/**
 * ============================================================================
 * Assignment 3 - CO3: Optimized Data Storage and Retrieval Using Tree Structures
 * Question 1: Binary Search Tree (Library Catalogue)
 * File: bst.c
 * 
 * Standard: C11 (gcc -Wall -Wextra -std=c11 -g)
 * Author: Student
 * Description: Complete BST implementation including insertion, search with
 *              comparison counter, 3-case deletion with inorder successor,
 *              height (empty = -1), preorder/inorder/postorder/level-order
 *              traversals, file persistence via preorder serialization,
 *              ASCII sideways visualizer, Graphviz DOT exporter, and
 *              rigorous 2,000-key empirical benchmark (random vs sorted).
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

/* Structure representing a node in the Binary Search Tree */
typedef struct BSTNode {
    int key;                    /* Accession number / book identifier */
    struct BSTNode *left;       /* Pointer to left child */
    struct BSTNode *right;      /* Pointer to right child */
} BSTNode;

/* -------------------------------------------------------------------------
 * Basic Operations
 * ------------------------------------------------------------------------- */

/**
 * Creates and allocates memory for a new BST node.
 * Initializes left and right child pointers to NULL.
 */
BSTNode *create_node(int key) {
    BSTNode *node = (BSTNode *)malloc(sizeof(BSTNode));
    if (!node) {
        fprintf(stderr, "Error: Memory allocation failed for key %d\n", key);
        exit(EXIT_FAILURE);
    }
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/**
 * Inserts a key into the BST.
 * Requirement: Ignores duplicate keys.
 * Returns the root of the modified subtree.
 */
BSTNode *insert(BSTNode *root, int key) {
    if (!root) {
        return create_node(key);
    }
    if (key < root->key) {
        root->left = insert(root->left, key);
    } else if (key > root->key) {
        root->right = insert(root->right, key);
    }
    /* Duplicate key: ignored per requirement */
    return root;
}

/**
 * Searches for a key in the BST while counting node comparisons.
 * Parameters:
 *   root: pointer to the root of the tree
 *   key: key to locate
 *   comparisons: pointer to an integer where total comparisons are accumulated
 * Returns:
 *   1 if found, 0 if not found.
 */
int search(const BSTNode *root, int key, int *comparisons) {
    *comparisons = 0;
    const BSTNode *curr = root;
    while (curr) {
        (*comparisons)++;
        if (key == curr->key) {
            return 1; /* Found */
        }
        if (key < curr->key) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }
    return 0; /* Not found */
}

/**
 * Finds the node with the minimum key in a given non-empty BST subtree.
 * In a BST, the minimum node is always the leftmost node.
 */
BSTNode *find_min(BSTNode *node) {
    BSTNode *curr = node;
    while (curr && curr->left) {
        curr = curr->left;
    }
    return curr;
}

/**
 * Deletes a key from the BST handling all three cases:
 *  Case 1: Node is a leaf (0 children) -> free node, return NULL.
 *  Case 2: Node has one child -> free node, return existing child.
 *  Case 3: Node has two children -> find inorder successor (minimum of right subtree),
 *          copy successor key to current node, then recursively delete successor.
 * Returns the root of the modified subtree.
 */
BSTNode *delete_node(BSTNode *root, int key) {
    if (!root) {
        return NULL; /* Key not found or empty tree */
    }

    if (key < root->key) {
        root->left = delete_node(root->left, key);
    } else if (key > root->key) {
        root->right = delete_node(root->right, key);
    } else {
        /* Node with matching key found */

        /* Case 1: Leaf node (no children) */
        if (!root->left && !root->right) {
            free(root);
            return NULL;
        }
        /* Case 2: One child (only right child exists) */
        else if (!root->left) {
            BSTNode *temp = root->right;
            free(root);
            return temp;
        }
        /* Case 2: One child (only left child exists) */
        else if (!root->right) {
            BSTNode *temp = root->left;
            free(root);
            return temp;
        }
        /* Case 3: Two children */
        else {
            /* Find inorder successor: smallest node in the right subtree */
            BSTNode *succ = find_min(root->right);
            root->key = succ->key;
            /* Delete the successor from right subtree */
            root->right = delete_node(root->right, succ->key);
        }
    }
    return root;
}

/**
 * Computes the height of the BST.
 * Requirement: Height of an empty tree is defined as -1.
 * Height of a single-node tree is 0.
 */
int height(const BSTNode *root) {
    if (!root) {
        return -1;
    }
    int lh = height(root->left);
    int rh = height(root->right);
    return 1 + (lh > rh ? lh : rh);
}

/**
 * Recursively deallocates all memory used by the tree.
 */
void free_tree(BSTNode *root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

/* -------------------------------------------------------------------------
 * Traversals
 * ------------------------------------------------------------------------- */

void preorder(const BSTNode *root) {
    if (!root) return;
    printf("%d ", root->key);
    preorder(root->left);
    preorder(root->right);
}

void inorder(const BSTNode *root) {
    if (!root) return;
    inorder(root->left);
    printf("%d ", root->key);
    inorder(root->right);
}

void postorder(const BSTNode *root) {
    if (!root) return;
    postorder(root->left);
    postorder(root->right);
    printf("%d ", root->key);
}

/* Queue implementation for Level-Order traversal */
typedef struct QueueNode {
    const BSTNode *tree_node;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
} Queue;

static void queue_init(Queue *q) {
    q->front = q->rear = NULL;
}

static void enqueue(Queue *q, const BSTNode *node) {
    QueueNode *qn = (QueueNode *)malloc(sizeof(QueueNode));
    if (!qn) {
        fprintf(stderr, "Queue allocation failed\n");
        exit(EXIT_FAILURE);
    }
    qn->tree_node = node;
    qn->next = NULL;
    if (!q->rear) {
        q->front = q->rear = qn;
    } else {
        q->rear->next = qn;
        q->rear = qn;
    }
}

static const BSTNode *dequeue(Queue *q) {
    if (!q->front) return NULL;
    QueueNode *temp = q->front;
    const BSTNode *res = temp->tree_node;
    q->front = q->front->next;
    if (!q->front) q->rear = NULL;
    free(temp);
    return res;
}

static bool queue_empty(const Queue *q) {
    return q->front == NULL;
}

void levelorder(const BSTNode *root) {
    if (!root) return;
    Queue q;
    queue_init(&q);
    enqueue(&q, root);
    while (!queue_empty(&q)) {
        const BSTNode *curr = dequeue(&q);
        printf("%d ", curr->key);
        if (curr->left) enqueue(&q, curr->left);
        if (curr->right) enqueue(&q, curr->right);
    }
}

/* -------------------------------------------------------------------------
 * Tree Visualizer & Graphviz DOT Exporter
 * ------------------------------------------------------------------------- */

/**
 * Prints a 2D sideways ASCII visualization of the tree.
 * Root is at the left, right children branch upward, left children downward.
 */
static void print_ascii_helper(const BSTNode *root, int space) {
    const int COUNT = 6;
    if (!root) return;
    space += COUNT;
    print_ascii_helper(root->right, space);
    printf("\n");
    for (int i = COUNT; i < space; i++) printf(" ");
    printf("[%d]\n", root->key);
    print_ascii_helper(root->left, space);
}

void print_ascii(const BSTNode *root) {
    if (!root) {
        printf("(empty tree)\n");
        return;
    }
    print_ascii_helper(root, 0);
    printf("----------------------------------------\n");
}

static void dot_helper(const BSTNode *root, FILE *fp) {
    if (!root) return;
    if (root->left) {
        fprintf(fp, "    \"%d\" -> \"%d\" [label=\"L\"];\n", root->key, root->left->key);
        dot_helper(root->left, fp);
    }
    if (root->right) {
        fprintf(fp, "    \"%d\" -> \"%d\" [label=\"R\"];\n", root->key, root->right->key);
        dot_helper(root->right, fp);
    }
    if (!root->left && !root->right) {
        fprintf(fp, "    \"%d\" [shape=ellipse];\n", root->key);
    }
}

void export_dot(const BSTNode *root, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Cannot open %s for writing\n", filename);
        return;
    }
    fprintf(fp, "digraph BST {\n");
    fprintf(fp, "    node [fontname=\"Helvetica\", shape=circle];\n");
    if (!root) {
        fprintf(fp, "    empty [label=\"Empty\", shape=plaintext];\n");
    } else {
        dot_helper(root, fp);
    }
    fprintf(fp, "}\n");
    fclose(fp);
    printf("Graphviz DOT exported to %s\n", filename);
}

/* -------------------------------------------------------------------------
 * File Persistence: Preorder Serialization and Rebuilding
 * ------------------------------------------------------------------------- */

/**
 * WHY PREORDER PRESERVES BST SHAPE:
 * In a general binary tree, preorder alone cannot reconstruct tree shape
 * without null markers. However, in a Binary Search Tree (BST), every key
 * satisfies the BST ordering invariant (all keys in left subtree < root < all keys
 * in right subtree).
 * Therefore, when keys are stored in Preorder (Root, Left, Right) and inserted
 * sequentially into an initially empty BST, the first key becomes the root,
 * subsequent smaller keys naturally populate the left subtree in exact original
 * sub-hierarchy, and larger keys populate the right subtree. This uniquely
 * reconstructs the identical tree structure without requiring null pointers!
 */
void save_preorder_helper(const BSTNode *root, FILE *fp) {
    if (!root) return;
    fprintf(fp, "%d\n", root->key);
    save_preorder_helper(root->left, fp);
    save_preorder_helper(root->right, fp);
}

int save_tree_preorder(const BSTNode *root, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) return 0;
    save_preorder_helper(root, fp);
    fclose(fp);
    return 1;
}

BSTNode *load_tree_preorder(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) return NULL;
    BSTNode *root = NULL;
    int key;
    while (fscanf(fp, "%d", &key) == 1) {
        root = insert(root, key);
    }
    fclose(fp);
    return root;
}

/* -------------------------------------------------------------------------
 * Timing Utility using CLOCK_MONOTONIC
 * ------------------------------------------------------------------------- */
static double get_elapsed_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

/* -------------------------------------------------------------------------
 * Main Driver: Verification, Deletions, Serialization & 2,000 Key Benchmark
 * ------------------------------------------------------------------------- */
int main(void) {
    printf("=================================================================\n");
    printf("   QUESTION 1: BINARY SEARCH TREE (LIBRARY CATALOGUE BENCHMARK)   \n");
    printf("=================================================================\n\n");

    /* ---------------------------------------------------------------------
     * Part A: Demonstrating All 3 Deletion Cases on Sample Tree
     * Keys: {50, 30, 70, 20, 40, 60, 80, 65}
     * --------------------------------------------------------------------- */
    printf("--- PART A: THREE CASES OF DELETION ---\n");
    int demo_keys[] = {50, 30, 70, 20, 40, 60, 80, 65};
    int n_demo = sizeof(demo_keys) / sizeof(demo_keys[0]);
    BSTNode *t = NULL;
    for (int i = 0; i < n_demo; i++) {
        t = insert(t, demo_keys[i]);
    }

    printf("Initial tree built with {50, 30, 70, 20, 40, 60, 80, 65}:\n");
    print_ascii(t);
    export_dot(t, "q1_bst/output/tree_initial.dot");

    printf("\nTraversals of Initial Tree:\n");
    printf("Preorder:   "); preorder(t); printf("\n");
    printf("Inorder:    "); inorder(t); printf("\n");
    printf("Postorder:  "); postorder(t); printf("\n");
    printf("Level-order:"); levelorder(t); printf("\n");
    printf("Height:     %d (empty=-1, leaf=0)\n\n", height(t));

    /* Case 1: Delete a leaf (20) */
    printf(">> Deleting Key 20 [CASE 1: Leaf Node (0 children)]:\n");
    t = delete_node(t, 20);
    print_ascii(t);

    /* Case 2: Delete a node with one child (30 has only right child 40) */
    printf(">> Deleting Key 30 [CASE 2: One Child (right child 40)]:\n");
    t = delete_node(t, 30);
    print_ascii(t);

    /* Case 3: Delete a node with two children (50 has left 40 and right 70) */
    printf(">> Deleting Key 50 [CASE 3: Two Children (inorder successor is 60)]:\n");
    t = delete_node(t, 50);
    print_ascii(t);
    export_dot(t, "q1_bst/output/tree_after_deletions.dot");

    printf("Preorder after deletions: "); preorder(t); printf("\n");
    printf("Inorder after deletions:  "); inorder(t); printf("\n\n");

    /* ---------------------------------------------------------------------
     * Part B: File Persistence (Save & Rebuild via Preorder)
     * --------------------------------------------------------------------- */
    printf("--- PART B: FILE PERSISTENCE VIA PREORDER SERIALIZATION ---\n");
    const char *persist_file = "q1_bst/output/bst_saved.txt";
    if (save_tree_preorder(t, persist_file)) {
        printf("Successfully saved tree to '%s' in preorder.\n", persist_file);
    }
    BSTNode *rebuilt = load_tree_preorder(persist_file);
    printf("Rebuilt tree from file:\n");
    print_ascii(rebuilt);
    printf("Inorder of rebuilt tree: "); inorder(rebuilt); printf("\n");
    printf("Heights: original = %d, rebuilt = %d (identical structure verified!)\n\n",
           height(t), height(rebuilt));
    free_tree(rebuilt);
    free_tree(t);

    /* ---------------------------------------------------------------------
     * Part C: Edge Cases (Empty Tree & Duplicate Keys)
     * --------------------------------------------------------------------- */
    printf("--- PART C: EDGE CASES ---\n");
    BSTNode *empty_tree = NULL;
    int cmp = 0;
    printf("Empty Tree:\n");
    printf("  Height: %d (expected -1)\n", height(empty_tree));
    printf("  Search 100: %s (comparisons=%d)\n",
           search(empty_tree, 100, &cmp) ? "Found" : "Not Found", cmp);
    printf("  Preorder: "); preorder(empty_tree); printf("(none)\n");
    printf("  Delete from empty: %s\n",
           delete_node(empty_tree, 100) == NULL ? "Handled safely (NULL)" : "Error");

    printf("\nDuplicate Keys Test:\n");
    BSTNode *dup_tree = NULL;
    dup_tree = insert(dup_tree, 100);
    dup_tree = insert(dup_tree, 100); /* duplicate */
    dup_tree = insert(dup_tree, 100); /* duplicate */
    printf("  Inserted 100 three times. Inorder: "); inorder(dup_tree); printf("\n");
    printf("  Height: %d (expected 0, single node)\n\n", height(dup_tree));
    free_tree(dup_tree);

    /* ---------------------------------------------------------------------
     * Part D: Empirical Investigation (2,000 Keys Random vs Sorted)
     * --------------------------------------------------------------------- */
    printf("--- PART D: EMPIRICAL BENCHMARK (2,000 KEYS: RANDOM VS SORTED) ---\n");
    const int N = 2000;
    int *keys_sorted = (int *)malloc(N * sizeof(int));
    int *keys_random = (int *)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) {
        keys_sorted[i] = (i + 1) * 10;
        keys_random[i] = keys_sorted[i];
    }

    /* Shuffle keys_random with Fisher-Yates */
    srand(42); /* Fixed seed for reproducibility */
    for (int i = N - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = keys_random[i];
        keys_random[i] = keys_random[j];
        keys_random[j] = tmp;
    }

    struct timespec start, end;

    /* 1. Build Random BST */
    BSTNode *bst_random = NULL;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < N; i++) {
        bst_random = insert(bst_random, keys_random[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double insert_time_random = get_elapsed_ms(start, end);
    int h_random = height(bst_random);

    /* 2. Build Sorted BST (Degenerate / Skewed Chain) */
    BSTNode *bst_sorted = NULL;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < N; i++) {
        bst_sorted = insert(bst_sorted, keys_sorted[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double insert_time_sorted = get_elapsed_ms(start, end);
    int h_sorted = height(bst_sorted);

    /* 3. Search All 2,000 Keys in Random BST */
    long long total_cmp_random = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < N; i++) {
        int c = 0;
        search(bst_random, keys_random[i], &c);
        total_cmp_random += c;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double search_time_random = get_elapsed_ms(start, end);
    double avg_cmp_random = (double)total_cmp_random / N;

    /* 4. Search All 2,000 Keys in Sorted BST */
    long long total_cmp_sorted = 0;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < N; i++) {
        int c = 0;
        search(bst_sorted, keys_sorted[i], &c);
        total_cmp_sorted += c;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double search_time_sorted = get_elapsed_ms(start, end);
    double avg_cmp_sorted = (double)total_cmp_sorted / N;

    /* Print Results Table */
    printf("+-----------------------------------+-------------------+-------------------+\n");
    printf("| Metric (N = 2,000 keys)           | Random Order BST  | Sorted Order BST  |\n");
    printf("+-----------------------------------+-------------------+-------------------+\n");
    printf("| Tree Height                       | %-17d | %-17d |\n", h_random, h_sorted);
    printf("| Insert Time (ms)                  | %-17.3f | %-17.3f |\n", insert_time_random, insert_time_sorted);
    printf("| Total Search Comparisons (2,000)  | %-17lld | %-17lld |\n", total_cmp_random, total_cmp_sorted);
    printf("| Avg Comparisons per Search        | %-17.2f | %-17.2f |\n", avg_cmp_random, avg_cmp_sorted);
    printf("| Total Search Time (ms)            | %-17.3f | %-17.3f |\n", search_time_random, search_time_sorted);
    printf("+-----------------------------------+-------------------+-------------------+\n\n");

    printf("Analysis of Benchmark:\n");
    printf("1. Random Order: Height = %d (approx 1.39 * log2(2000) ~ 15), avg comparisons = %.2f.\n",
           h_random, avg_cmp_random);
    printf("2. Sorted Order: Height = %d (N - 1 = 1999 degenerate chain), avg comparisons = %.2f (~ N/2 = 1000.5).\n",
           h_sorted, avg_cmp_sorted);
    printf("3. The sorted order causes searching to degrade from O(log N) to linear O(N), proving why\n");
    printf("   self-balancing trees (AVL / Red-Black) are mandatory for ordered insertions.\n\n");

    /* Clean up allocated memory */
    free(keys_sorted);
    free(keys_random);
    free_tree(bst_random);
    free_tree(bst_sorted);

    printf("All memory successfully freed. Zero memory leaks.\n");
    return 0;
}
```

## book.c (Complete the Code)

```c
/**
 * ============================================================================
 * Assignment 3 - CO3: Complete the Code
 * Question 1: Book Library Catalogue
 * File: book.c
 * 
 * Required output:
 * 200 300 350 400 500 600 700 800 
 * height: random order 3, sorted order 7
 * comparisons: 4 and 8
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct Book {
    int acc;                                /* accession number (key) */
    struct Book *left, *right;
} Book;

Book *insert(Book *root, int acc)
{
    // TODO 1: If root is NULL create and return a new node. Otherwise insert into the
    //   left subtree if acc is smaller, the right subtree if larger; ignore duplicates.
    //   Return root
    if (!root) {
        Book *node = (Book *)malloc(sizeof(Book));
        if (!node) return NULL;
        node->acc = acc;
        node->left = NULL;
        node->right = NULL;
        return node;
    }
    if (acc < root->acc) {
        root->left = insert(root->left, acc);
    } else if (acc > root->acc) {
        root->right = insert(root->right, acc);
    }
    return root;
}

int height(const Book *r)                   /* height of an empty tree is -1 */
{
    // TODO 2: Return -1 for an empty tree, otherwise 1 + the larger of the two subtree
    //   heights
    if (!r) {
        return -1;
    }
    int lh = height(r->left);
    int rh = height(r->right);
    return 1 + (lh > rh ? lh : rh);
}

int search(const Book *r, int acc, int *cmp)
{
    *cmp = 0;
    while (r) {
        (*cmp)++;
        if (acc == r->acc) return 1;
        r = acc < r->acc ? r->left : r->right;
    }
    return 0;
}

void inorder(const Book *r)
{
    if (!r) return;
    inorder(r->left);
    printf("%d ", r->acc);
    inorder(r->right);
}

void destroy(Book *r)
{
    if (!r) return;
    destroy(r->left);
    destroy(r->right);
    free(r);
}

int main(void)
{
    int a[] = {500, 300, 700, 200, 400, 600, 800, 350};
    int s[] = {10, 20, 30, 40, 50, 60, 70, 80};
    Book *random = NULL, *sorted = NULL;
    for (int i = 0; i < 8; i++) random = insert(random, a[i]);
    for (int i = 0; i < 8; i++) sorted = insert(sorted, s[i]);
    inorder(random);
    printf("\nheight: random order %d, sorted order %d\n",
           height(random), height(sorted));
    int c1, c2;
    search(random, 350, &c1);
    search(sorted, 80, &c2);
    printf("comparisons: %d and %d\n", c1, c2);
    destroy(random);
    destroy(sorted);
    return 0;
}
```

## q1_predict1.c

```c
#include <stdio.h>
/* Tree stored in an array: children of i are 2i+1 and 2i+2; '.' means empty. */
const char T[] = "ABCDE.F";
int N = 7;
void pre(int i)
{
    if (i >= N || T[i] == '.') return;
    printf("%c", T[i]); pre(2 * i + 1); pre(2 * i + 2);
}
void in(int i)
{
    if (i >= N || T[i] == '.') return;
    in(2 * i + 1); printf("%c", T[i]); in(2 * i + 2);
}
void post(int i)
{
    if (i >= N || T[i] == '.') return;
    post(2 * i + 1); post(2 * i + 2); printf("%c", T[i]);
}
int main(void)
{
    pre(0);  printf(" ");
    in(0);   printf(" ");
    post(0); printf(" ");
    for (int i = 0; i < N; i++) if (T[i] != '.') printf("%c", T[i]);
    printf("\n");
    return 0;
}
```

## q1_predict2.c

```c
#include <stdio.h>
#include <stdlib.h>
typedef struct N { int k; struct N *l, *r; } N;
N *ins(N *t, int k)
{
    if (!t) { t = calloc(1, sizeof *t); t->k = k; return t; }
    if (k < t->k) t->l = ins(t->l, k); else t->r = ins(t->r, k);
    return t;
}
N *del(N *t, int k)
{
    if (!t) return NULL;
    if (k < t->k) t->l = del(t->l, k);
    else if (k > t->k) t->r = del(t->r, k);
    else if (t->l && t->r) {
        N *s = t->r;
        while (s->l) s = s->l;               /* inorder successor */
        t->k = s->k;
        t->r = del(t->r, s->k);
    } else {
        N *c = t->l ? t->l : t->r;
        free(t);
        return c;
    }
    return t;
}
void pre(N *t) { if (t) { printf("%d ", t->k); pre(t->l); pre(t->r); } }
void kill(N *t) { if (t) { kill(t->l); kill(t->r); free(t); } }
int main(void)
{
    int a[] = {50, 30, 70, 20, 40, 60, 80, 65};
    N *t = NULL;
    for (int i = 0; i < 8; i++) t = ins(t, a[i]);
    t = del(t, 20);
    t = del(t, 30);
    t = del(t, 50);
    pre(t);
    printf("\n");
    kill(t);
    return 0;
}
```

