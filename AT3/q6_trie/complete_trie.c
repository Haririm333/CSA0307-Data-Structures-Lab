/**
 * ============================================================================
 * Assignment 3 - CO3: Complete the Code
 * Question 6: Trie - Phone Contact Search & Autocomplete
 * File: complete_trie.c
 * 
 * Required output:
 * a:5 an:4 anu:2 b:1 z:0 
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct Trie {
    struct Trie *child[26];
    int words;                              /* number of words passing through here */
    int end;                                /* 1 if a word ends here */
} Trie;

static int g_node_count = 0;

Trie *node(void) {
    g_node_count++;
    return calloc(1, sizeof(Trie));
}

void insert(Trie *t, const char *w)
{
    // TODO 1: For every lowercase letter of w, create the child node if it is missing,
    //   move to it and increase its words count; mark end at the last node
    Trie *curr = t;
    for (int i = 0; w[i] != '\0'; i++) {
        int idx = w[i] - 'a';
        if (idx < 0 || idx >= 26) continue;
        if (!curr->child[idx]) {
            curr->child[idx] = node();
        }
        curr = curr->child[idx];
        curr->words++;
    }
    curr->end = 1;
}

int count_prefix(const Trie *t, const char *p)
{
    // TODO 2: Follow the letters of p; return 0 if a letter is missing, otherwise the
    //   words count of the last node
    const Trie *curr = t;
    for (int i = 0; p[i] != '\0'; i++) {
        int idx = p[i] - 'a';
        if (idx < 0 || idx >= 26) return 0;
        if (!curr->child[idx]) {
            return 0;
        }
        curr = curr->child[idx];
    }
    return curr->words;
}

void destroy(Trie *t)
{
    if (!t) return;
    for (int i = 0; i < 26; i++) destroy(t->child[i]);
    free(t);
}

int main(void)
{
    const char *contacts[] = {"anand", "anitha", "anu", "arun", "bala", "anusuya"};
    Trie *root = node();
    for (int i = 0; i < 6; i++) insert(root, contacts[i]);
    const char *typed[] = {"a", "an", "anu", "b", "z"};
    for (int i = 0; i < 5; i++)
        printf("%s:%d ", typed[i], count_prefix(root, typed[i]));
    printf("\n");

    /* Memory and node analysis */
    printf("\n--- Contact Trie Node & Memory Calculation ---\n");
    printf("Total Trie nodes created: %d (including root)\n", g_node_count);
    printf("Size of single Trie node: %zu bytes (26 pointers * %zu + 2 ints * %zu)\n",
           sizeof(Trie), sizeof(void *), sizeof(int));
    printf("Total memory consumed:    %zu bytes (%.2f KB)\n",
           g_node_count * sizeof(Trie), (double)(g_node_count * sizeof(Trie)) / 1024.0);

    destroy(root);
    return 0;
}
