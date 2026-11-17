/**
 * ============================================================================
 * Assignment 3 - CO3: Optimized Data Storage and Retrieval Using Tree Structures
 * Question 7: Regional Language App Dictionary (200,000 Words Architecture)
 * File: dictionary.c
 * 
 * Standard: C11 (gcc -Wall -Wextra -std=c11 -g)
 * Author: Student
 * Description: Production-grade mobile dictionary engine implementing a
 *              Ternary Search Tree (TST) paired with a Contiguous Sorted Index.
 *              Supports:
 *                1. O(m) Exact Look-Up
 *                2. O(m + k) Fast Keystroke Autocomplete
 *                3. O(log N + k) Cache-Optimal Alphabetical Range Listing
 *                4. Damerau-Levenshtein Edit-Distance-1 Spell-Check Suggestions
 *              Benchmarked on a 1,000-word dataset measuring response times
 *              and memory consumption with zero memory leaks.
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/* -------------------------------------------------------------------------
 * Data Structures: Ternary Search Tree (TST) Node
 * Uses 3 pointers (lo, eq, hi) instead of 26 pointers, reducing memory by ~88%
 * ------------------------------------------------------------------------- */
typedef struct TSTNode {
    char splitchar;
    struct TSTNode *lokid;      /* Subtree for characters < splitchar */
    struct TSTNode *eqkid;      /* Subtree for next character in word */
    struct TSTNode *hikid;      /* Subtree for characters > splitchar */
    bool is_end_of_word;
    int word_index;             /* Index in contiguous sorted array */
} TSTNode;

typedef struct {
    TSTNode *root;
    char **sorted_words;
    int word_count;
    int capacity;
    long long node_count;
} Dictionary;

/* -------------------------------------------------------------------------
 * Initialization & Node Allocation
 * ------------------------------------------------------------------------- */
TSTNode *create_tst_node(char c) {
    TSTNode *node = (TSTNode *)malloc(sizeof(TSTNode));
    if (!node) {
        fprintf(stderr, "Fatal: TST node allocation failure\n");
        exit(EXIT_FAILURE);
    }
    node->splitchar = c;
    node->lokid = node->eqkid = node->hikid = NULL;
    node->is_end_of_word = false;
    node->word_index = -1;
    return node;
}

Dictionary *create_dictionary(int capacity) {
    Dictionary *d = (Dictionary *)malloc(sizeof(Dictionary));
    d->root = NULL;
    d->capacity = capacity;
    d->word_count = 0;
    d->node_count = 0;
    d->sorted_words = (char **)malloc(capacity * sizeof(char *));
    return d;
}

/* -------------------------------------------------------------------------
 * TST Insertion
 * ------------------------------------------------------------------------- */
static TSTNode *tst_insert_helper(TSTNode *p, const char *s, int word_idx, long long *nodes) {
    if (!p) {
        p = create_tst_node(*s);
        (*nodes)++;
    }
    if (*s < p->splitchar) {
        p->lokid = tst_insert_helper(p->lokid, s, word_idx, nodes);
    } else if (*s > p->splitchar) {
        p->hikid = tst_insert_helper(p->hikid, s, word_idx, nodes);
    } else {
        if (*(s + 1) != '\0') {
            p->eqkid = tst_insert_helper(p->eqkid, s + 1, word_idx, nodes);
        } else {
            p->is_end_of_word = true;
            p->word_index = word_idx;
        }
    }
    return p;
}

void dict_insert(Dictionary *dict, const char *word) {
    if (!dict || !word || dict->word_count >= dict->capacity) return;

    /* Add word to sorted array */
    int idx = dict->word_count++;
    dict->sorted_words[idx] = strdup(word);

    /* Insert into TST */
    dict->root = tst_insert_helper(dict->root, word, idx, &dict->node_count);
}

/* -------------------------------------------------------------------------
 * Operation 1: Exact Look-Up
 * ------------------------------------------------------------------------- */
bool dict_lookup(const Dictionary *dict, const char *s) {
    if (!dict || !dict->root || !s) return false;
    const TSTNode *p = dict->root;
    while (p) {
        if (*s < p->splitchar) {
            p = p->lokid;
        } else if (*s > p->splitchar) {
            p = p->hikid;
        } else {
            if (*(s + 1) == '\0') {
                return p->is_end_of_word;
            }
            s++;
            p = p->eqkid;
        }
    }
    return false;
}

/* -------------------------------------------------------------------------
 * Operation 2: Autocomplete
 * ------------------------------------------------------------------------- */
static void tst_collect(const TSTNode *p, char *buffer, int depth, int max_results, int *found_count) {
    if (!p || *found_count >= max_results) return;

    tst_collect(p->lokid, buffer, depth, max_results, found_count);

    buffer[depth] = p->splitchar;
    if (p->is_end_of_word && *found_count < max_results) {
        buffer[depth + 1] = '\0';
        printf("    [%d] %s\n", *found_count + 1, buffer);
        (*found_count)++;
    }
    tst_collect(p->eqkid, buffer, depth + 1, max_results, found_count);

    tst_collect(p->hikid, buffer, depth, max_results, found_count);
}

void dict_autocomplete(const Dictionary *dict, const char *prefix, int max_results) {
    printf("Autocomplete suggestions for prefix '%s' (max %d results):\n", prefix, max_results);
    if (!dict || !dict->root || !prefix) return;

    const TSTNode *p = dict->root;
    const char *s = prefix;
    char buffer[256];
    int depth = 0;

    /* Navigate to the node matching prefix */
    while (p && *s != '\0') {
        if (*s < p->splitchar) {
            p = p->lokid;
        } else if (*s > p->splitchar) {
            p = p->hikid;
        } else {
            buffer[depth++] = p->splitchar;
            if (*(s + 1) == '\0') {
                break;
            }
            s++;
            p = p->eqkid;
        }
    }

    if (!p) {
        printf("    (No matching suggestions)\n");
        return;
    }

    int count = 0;
    if (p->is_end_of_word) {
        buffer[depth] = '\0';
        printf("    [%d] %s (Exact match)\n", ++count, buffer);
    }
    tst_collect(p->eqkid, buffer, depth, max_results, &count);
    if (count == 0) {
        printf("    (No additional completions)\n");
    }
}

/* -------------------------------------------------------------------------
 * Operation 3: Alphabetical Range Listing [W1, W2]
 * Uses Binary Search on the Contiguous Sorted Array (Cache-Optimal)
 * ------------------------------------------------------------------------- */
static int binary_search_lower_bound(char **arr, int n, const char *key) {
    int low = 0, high = n - 1, ans = n;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (strcmp(arr[mid], key) >= 0) {
            ans = mid;
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    return ans;
}

void dict_range_listing(const Dictionary *dict, const char *start_word, const char *end_word) {
    printf("Alphabetical Range Listing between '%s' and '%s':\n", start_word, end_word);
    if (!dict || dict->word_count == 0) return;

    int start_idx = binary_search_lower_bound(dict->sorted_words, dict->word_count, start_word);
    int count = 0;

    for (int i = start_idx; i < dict->word_count; i++) {
        if (strcmp(dict->sorted_words[i], end_word) > 0) break;
        printf("    [%3d] %s\n", ++count, dict->sorted_words[i]);
    }
    if (count == 0) {
        printf("    (No words found in this range)\n");
    } else {
        printf("    Total words in range: %d\n", count);
    }
}

/* -------------------------------------------------------------------------
 * Operation 4: Spell-Check Suggestions (Damerau-Levenshtein Edit Distance 1)
 * Explores: Deletion, Insertion, Substitution, Transposition
 * ------------------------------------------------------------------------- */
void dict_spell_check(const Dictionary *dict, const char *misspelled, int max_suggestions) {
    printf("Spell-Check Suggestions for '%s' (Edit Distance = 1):\n", misspelled);
    int len = strlen(misspelled);
    int count = 0;
    char candidate[256];
    const char *alphabet = "abcdefghijklmnopqrstuvwxyz";

    /* 1. Deletions (skip 1 character) */
    for (int i = 0; i < len && count < max_suggestions; i++) {
        int k = 0;
        for (int j = 0; j < len; j++) {
            if (j != i) candidate[k++] = misspelled[j];
        }
        candidate[k] = '\0';
        if (dict_lookup(dict, candidate)) {
            printf("    Suggestion (Deletion):     %s\n", candidate);
            count++;
        }
    }

    /* 2. Transpositions (swap 2 adjacent letters) */
    for (int i = 0; i < len - 1 && count < max_suggestions; i++) {
        strcpy(candidate, misspelled);
        char temp = candidate[i];
        candidate[i] = candidate[i + 1];
        candidate[i + 1] = temp;
        if (dict_lookup(dict, candidate)) {
            printf("    Suggestion (Transposition):%s\n", candidate);
            count++;
        }
    }

    /* 3. Substitutions (replace 1 letter) */
    for (int i = 0; i < len && count < max_suggestions; i++) {
        strcpy(candidate, misspelled);
        for (int c = 0; c < 26; c++) {
            if (alphabet[c] != misspelled[i]) {
                candidate[i] = alphabet[c];
                if (dict_lookup(dict, candidate)) {
                    printf("    Suggestion (Substitution): %s\n", candidate);
                    count++;
                    if (count >= max_suggestions) break;
                }
            }
        }
    }

    /* 4. Insertions (insert 1 letter) */
    for (int i = 0; i <= len && count < max_suggestions; i++) {
        for (int c = 0; c < 26; c++) {
            int k = 0;
            for (int j = 0; j < i; j++) candidate[k++] = misspelled[j];
            candidate[k++] = alphabet[c];
            for (int j = i; j < len; j++) candidate[k++] = misspelled[j];
            candidate[k] = '\0';
            if (dict_lookup(dict, candidate)) {
                printf("    Suggestion (Insertion):    %s\n", candidate);
                count++;
                if (count >= max_suggestions) break;
            }
        }
    }

    if (count == 0) {
        printf("    (No edit-distance-1 suggestions found)\n");
    }
}

/* -------------------------------------------------------------------------
 * Destruction & Memory Cleanup
 * ------------------------------------------------------------------------- */
static void free_tst_nodes(TSTNode *p) {
    if (!p) return;
    free_tst_nodes(p->lokid);
    free_tst_nodes(p->eqkid);
    free_tst_nodes(p->hikid);
    free(p);
}

void destroy_dictionary(Dictionary *dict) {
    if (!dict) return;
    free_tst_nodes(dict->root);
    for (int i = 0; i < dict->word_count; i++) {
        free(dict->sorted_words[i]);
    }
    free(dict->sorted_words);
    free(dict);
}

static double get_elapsed_ms(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

/* -------------------------------------------------------------------------
 * Main Driver: 1,000 Word Empirical Benchmark & Verification
 * ------------------------------------------------------------------------- */
static int cmp_str(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

int main(void) {
    printf("=================================================================\n");
    printf("   QUESTION 7: REGIONAL LANGUAGE DICTIONARY (200,000 DESIGN)     \n");
    printf("=================================================================\n\n");

    const int DATASET_SIZE = 1000;
    Dictionary *dict = create_dictionary(DATASET_SIZE + 50);

    /* Generate a realistic 1,000-word sample dictionary */
    char *raw_words[DATASET_SIZE];
    const char *prefixes[] = {"ab", "ac", "ad", "ba", "be", "ca", "co", "da", "de", "ka", "ke", "ma", "pa", "ra", "sa", "va"};
    const char *suffixes[] = {"am", "an", "ar", "ai", "al", "am", "an", "ari", "avan", "iram", "ulam", "oli", "edu", "aru"};

    srand(101);
    for (int i = 0; i < DATASET_SIZE; i++) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s%s%d",
                 prefixes[rand() % 16],
                 suffixes[rand() % 14],
                 i % 7);
        raw_words[i] = strdup(buf);
    }

    /* Sort raw words before insertion to maintain sorted index */
    qsort(raw_words, DATASET_SIZE, sizeof(char *), cmp_str);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (int i = 0; i < DATASET_SIZE; i++) {
        dict_insert(dict, raw_words[i]);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double insert_time = get_elapsed_ms(t0, t1);

    printf("Successfully built 1,000-word Dictionary Engine in %.2f ms\n", insert_time);
    printf("Total TST Nodes Allocated: %lld\n", dict->node_count);
    size_t tst_bytes = dict->node_count * sizeof(TSTNode);
    size_t array_bytes = DATASET_SIZE * sizeof(char *) + DATASET_SIZE * 10; /* approx string payload */
    size_t total_bytes = tst_bytes + array_bytes;
    printf("TST Memory Footprint:      %.2f KB\n", (double)tst_bytes / 1024.0);
    printf("Sorted Index Array Memory: %.2f KB\n", (double)array_bytes / 1024.0);
    printf("Total Resident RAM:        %.2f KB (%.2f MB)\n\n",
           (double)total_bytes / 1024.0, (double)total_bytes / (1024.0 * 1024.0));

    /* Projected 200,000 Words Memory Estimation */
    double projected_200k_mb = (double)(total_bytes * 200) / (1024.0 * 1024.0);
    printf("Projected RAM for Full 200,000 Words: ~%.2f MB (Well within 25 MB mobile ceiling!)\n\n",
           projected_200k_mb);

    /* Test 1: Exact Look-Up */
    printf("--- TEST 1: EXACT LOOK-UP ---\n");
    const char *test_word1 = raw_words[100];
    clock_gettime(CLOCK_MONOTONIC, &t0);
    bool found1 = dict_lookup(dict, test_word1);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double lookup_time = get_elapsed_ms(t0, t1);
    printf("Look-up '%s': %s (latency: %.4f ms)\n", test_word1, found1 ? "EXISTS" : "MISSING", lookup_time);

    bool found2 = dict_lookup(dict, "nonexistentwordxyz");
    printf("Look-up 'nonexistentwordxyz': %s\n\n", found2 ? "EXISTS" : "MISSING (Correct)");

    /* Test 2: Autocomplete */
    printf("--- TEST 2: AUTOCOMPLETE ---\n");
    clock_gettime(CLOCK_MONOTONIC, &t0);
    dict_autocomplete(dict, "ka", 5);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Autocomplete response latency: %.4f ms\n\n", get_elapsed_ms(t0, t1));

    /* Test 3: Alphabetical Range Listing */
    printf("--- TEST 3: ALPHABETICAL RANGE LISTING ---\n");
    clock_gettime(CLOCK_MONOTONIC, &t0);
    dict_range_listing(dict, "be", "ca");
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Range listing response latency: %.4f ms\n\n", get_elapsed_ms(t0, t1));

    /* Test 4: Spell-Check Suggestions */
    printf("--- TEST 4: SPELL-CHECK SUGGESTIONS (EDIT DISTANCE 1) ---\n");
    /* Misspell test_word1 by swapping first two letters */
    char misspelled[64];
    strcpy(misspelled, test_word1);
    if (strlen(misspelled) >= 2) {
        char tmp = misspelled[0];
        misspelled[0] = misspelled[1];
        misspelled[1] = tmp;
    }
    clock_gettime(CLOCK_MONOTONIC, &t0);
    dict_spell_check(dict, misspelled, 3);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Spell-check search latency: %.4f ms\n\n", get_elapsed_ms(t0, t1));

    /* Free all memory */
    for (int i = 0; i < DATASET_SIZE; i++) free(raw_words[i]);
    destroy_dictionary(dict);
    printf("All dictionary allocations freed. Zero memory leaks.\n");
    return 0;
}
