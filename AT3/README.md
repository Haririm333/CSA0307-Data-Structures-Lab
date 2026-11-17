# Optimized Data Storage and Retrieval Using Tree-Based Structures
## Data Structures Assignment 3 (CO3) – SIMATS Engineering

A comprehensive, production-grade implementation of advanced tree-based data structures in standard C11. Every program is built strictly with the C standard library, fully commented for viva verification, rigorously tested for zero memory leaks, and benchmarked using monotonic high-resolution hardware timers (`clock_gettime(CLOCK_MONOTONIC)`).

---

## 1. Repository Directory Structure

```text
AT3/
├── Makefile                          # Unified build and automation script
├── README.md                         # Main repository guide, documentation & run instructions
├── CHECKLIST.md                      # Rubric cross-reference mapping items 1 to 87
├── generate_screenshots.py           # Script to render dark-mode console screenshots
├── output/                           # Consolidated outputs and global results
│   ├── results.md                    # Benchmark data tables across all questions
│   └── screenshots/                  # High-resolution generated screenshot PNGs
├── screenshots_placeholder/          # Rubric screenshot storage
│   ├── 01_q1_bst_terminal.png
│   ├── 02_q2_avl_terminal.png
│   ├── 03_q3_btree_terminal.png
│   ├── 04_q4_rbtree_terminal.png
│   ├── 05_q5_splay_terminal.png
│   ├── 06_q6_trie_terminal.png
│   └── 07_q7_dictionary_terminal.png
├── q1_bst/                           # Question 1: Binary Search Tree (Library Catalogue)
│   ├── bst.c                         # Complete BST, 3 deletion cases, traversals, 2k-key benchmark
│   ├── book.c                        # Completed code: insert() & height()
│   ├── q1_predict1.c                 # Predict the output 1 (array tree "ABCDE.F")
│   ├── q1_predict2.c                 # Predict the output 2 (BST deletion sequence)
│   ├── trace.md                      # Step-by-step traces, tree drawings & deletion cases
│   ├── analysis.md                   # Design & Analysis items 5-8, Conclusion, Alternative Concept
│   └── output/                       # Captured program outputs & Graphviz .dot files
├── q2_avl/                           # Question 2: AVL Tree (Live Stock Price Index)
│   ├── avl.c                         # Full AVL with 4 rotations & 100k sorted-key benchmark
│   ├── stock_avl.c                   # Completed code: rotate_right() & rotate_left()
│   ├── analysis.md                   # Fibonacci height proof, rotation drawings, alternatives
│   └── output/                       # Execution logs & step-by-step DOT files
├── q3_btree/                         # Question 3: B-Tree (Hospital Patient Database on Disk)
│   ├── btree.c                       # Order-4 (t=2) B-tree, node splits, disk read simulator
│   ├── analysis.md                   # Disk block sizing, B vs B+ tree, 5M patient disk seek analysis
│   └── output/                       # Output logs of insertions and disk read simulations
├── q4_rbtree/                        # Question 4: Red-Black Tree (Linux Process Scheduler / CFS)
│   ├── rbtree.c                      # Red-Black tree with sentinel NIL, cached leftmost, 10k benchmark
│   ├── analysis.md                   # 5 properties, proof that h <= 2 log(n+1), heap comparison
│   └── output/                       # Trace output and 5-property verification logs
├── q5_splay/                         # Question 5: Splay Tree (E-Commerce Frequently Viewed Products Cache)
│   ├── splay.c                       # Top-down splay (Zig, Zig-Zig, Zig-Zag), 80/5 Pareto benchmark
│   ├── analysis.md                   # Amortized O(log n) potential proof, LRU cache comparison
│   └── output/                       # Splay step logs and 10,000-access comparison table
├── q6_trie/                          # Question 6: Trie (Phone Contact Search & Autocomplete)
│   ├── trie.c                        # 26-way Trie, autocomplete, deletion, 50k names benchmark
│   ├── complete_trie.c               # Completed code: insert() & count_prefix()
│   ├── analysis.md                   # O(m) proof, memory analysis, radix compression, binary search
│   └── output/                       # Prefix search timings and contact autocompletions
└── q7_dictionary/                    # Question 7: Regional Language App Dictionary (200,000 Words)
    ├── dictionary.c                  # Ternary Search Tree (TST) + Sorted Index engine
    ├── design.md                     # System design stages 1 to 4 & hash table alternative
    ├── analysis.md                   # Architectural defense & 2-million-word scaling
    └── output/                       # Verified look-up, autocomplete, range listing & spell-check
```

---

## 2. Compilation and Execution Guide

### Prerequisites
- Compiler: GCC 11+ (`gcc -Wall -Wextra -std=c11 -g`)
- Make: GNU Make or MinGW32-Make
- Python 3 with `matplotlib` and `Pillow` (for screenshot generation)

### Unified Makefile Targets
From the root repository directory:
```bash
# 1. Compile all 7 questions
make all

# 2. Run all programs and record console logs to /output
make run

# 3. Clean all compiled binaries
make clean

# 4. Run question-specific targets
make q1      # Compile Q1 programs
make run_q1  # Run Q1 programs and save outputs
make q2      # Compile Q2 programs
make run_q2  # Run Q2 programs
make q3      # Compile Q3 B-Tree
make run_q3  # Run Q3 B-Tree
make q4      # Compile Q4 Red-Black Tree
make run_q4  # Run Q4 Red-Black Tree
make q5      # Compile Q5 Splay Tree
make run_q5  # Run Q5 Splay Tree
make q6      # Compile Q6 Trie
make run_q6  # Run Q6 Trie
make q7      # Compile Q7 Dictionary
make run_q7  # Run Q7 Dictionary
```

---

## 3. Question-by-Question Overview & Sample Outputs

### Question 1: Binary Search Tree (Library Catalogue)
- **Files**:
  - `q1_bst/bst.c`: Main driver with insertion (ignoring duplicates), comparison-counting search, 3-case deletion with inorder successor, height calculation (empty = -1), traversals (preorder, inorder, postorder, level-order with queue), ASCII visualization, DOT exporter, preorder serialization and rebuilding, and 2,000-key empirical benchmark.
  - `q1_bst/book.c`: Completed accession number catalogue program.
  - `q1_bst/q1_predict1.c` & `q1_bst/q1_predict2.c`: Exact assignment prediction programs.
  - `q1_bst/trace.md`: Full trace with tree drawings after every step.
  - `q1_bst/analysis.md`: Inorder sorted property proof, 3 deletion cases, height bounds, and sorted array comparison.
- **Compilation & Execution**:
  ```bash
  gcc -Wall -Wextra -std=c11 -g q1_bst/bst.c -o q1_bst/bst.exe
  ./q1_bst/bst.exe
  ```
- **Sample Output**:
  ```text
  200 300 350 400 500 600 700 800 
  height: random order 3, sorted order 7
  comparisons: 4 and 8
  
  +-----------------------------------+-------------------+-------------------+
  | Metric (N = 2,000 keys)           | Random Order BST  | Sorted Order BST  |
  +-----------------------------------+-------------------+-------------------+
  | Tree Height                       | 26                | 1999              |
  | Insert Time (ms)                  | 0.261             | 18.153            |
  | Total Search Comparisons (2,000)  | 28359             | 2001000           |
  | Avg Comparisons per Search        | 14.18             | 1000.50           |
  +-----------------------------------+-------------------+-------------------+
  ```

---

### Question 2: AVL Tree (Live Stock Price Index)
- **Files**:
  - `q2_avl/avl.c`: Implementation of the 4 rotations (LL, RR, LR, RL), height and balance tracking, step-by-step insertion of `{10, 20, 30, 40, 50, 25}`, `verify_avl()` validation across every node, and 100,000 sorted keys experiment.
  - `q2_avl/stock_avl.c`: Completed program with `rotate_right` and `rotate_left`.
  - `q2_avl/analysis.md`: Proof that $h < 1.44 \log_2(n+1)$, deletion rebalancing, and Red-Black/Skip-List trade-offs.
- **Sample Output**:
  ```text
  30 20 10 25 40 50 
  height = 3
  
  +---------------------------------------+-------------------+-------------------+
  | Metric (N = 100,000 keys)             | Plain BST (Sorted)| AVL Tree (Sorted) |
  +---------------------------------------+-------------------+-------------------+
  | Tree Height                           | 99999             | 17                |
  | Insert Time (ms)                      | 34434.046         | 18.311            |
  | Search Time (10,000 queries) (ms)     | 4699.304          | 1.026             |
  | Speedup Factor in Search              | 1.00x (baseline)  | 4580.2x           |
  | Balance Verification                  | N/A (Degenerate)  | 100% Valid        |
  +---------------------------------------+-------------------+-------------------+
  ```

---

### Question 3: B-Tree (Hospital Patient Database on Disk)
- **Files**:
  - `q3_btree/btree.c`: B-Tree of minimum degree $t = 2$ (Order 4), proactive top-down splitting, insertion with full split logging for `{10, 20, 5, 6, 12, 30, 7, 17}`, disk-read simulation tracking node accesses, and 5-million patient record disk I/O analysis.
  - `q3_btree/analysis.md`: Node splitting invariant, B vs B+ tree for range queries, and disk block packing formulas.
- **Sample Output**:
  ```text
  >> [STEP 4] Inserting Key: 6
      >>> [ROOT SPLIT] Root was full! Allocating new root; tree height increases by 1.
      >>> [SPLIT EVENT] Node containing keys [5, 10, 20] is FULL. Median key '10' promotes.
  
  +-------------------------------+---------------+---------------+-------------------+
  | Tree Structure                | Min Node Reads| Max Node Reads| Search Time @10ms |
  +-------------------------------+---------------+---------------+-------------------+
  | Plain BST (Worst Case)        | 5,000,000     | 5,000,000     | 50,000.00 seconds |
  | Plain BST (Average Random)    | 22            | 32            | 220 - 320 ms      |
  | AVL Tree (Guaranteed O(log N))| 23            | 33            | 230 - 330 ms      |
  | B-Tree (Order 100, t = 50)    | 4             | 4             | 40 - 40 ms        |
  +-------------------------------+---------------+---------------+-------------------+
  ```

---

### Question 4: Red-Black Tree (Linux Process Scheduler)
- **Files**:
  - `q4_rbtree/rbtree.c`: Complete Red-Black tree with sentinel NIL, recoloring/rotation fixup, cached leftmost node for $O(1)$ task dispatch, `check_properties()` validator for all 5 invariants, step trace of `{10, 20, 30, 15, 25, 5}`, and 10,000-operation random benchmark.
  - `q4_rbtree/analysis.md`: 5 properties, proof that $h \le 2 \log_2(n+1)$, rotation comparison, and binary min-heap trade-off.
- **Sample Output**:
  ```text
  Smallest Runnable Task (O(1) dispatch from cached leftmost): Process 5
  
  +---------------------------------------+-----------------------+
  | Red-Black Tree Experiment Metric      | Observed Value        |
  +---------------------------------------+-----------------------+
  | Height after 10,000 Random Inserts    | 16                    |
  | Insertion Rotations                   | 5855 (avg 0.586 <= 2) |
  | Recolor Events                        | 23186                 |
  | All 5 Properties after 10k Inserts    | 100% VALID            |
  | Height after 5,000 Random Deletions   | 16                    |
  | Deletion Rotations                    | 2198                  |
  | All 5 Properties after 5k Deletions   | 100% VALID            |
  +---------------------------------------+-----------------------+
  ```

---

### Question 5: Splay Tree (Product Cache)
- **Files**:
  - `q5_splay/splay.c`: Top-down splaying with Zig, Zig-Zig, and Zig-Zag operations, step-by-step splay demonstration for keys 3 and 9, 10,000-access simulation under an 80/5 Pareto distribution vs uniform workload compared to an AVL tree.
  - `q5_splay/analysis.md`: Amortized $O(\log n)$ potential function proof, zero balance metadata justification, and LRU cache comparison.
- **Sample Output**:
  ```text
  +---------------------------------------+-------------------+-------------------+
  | Access Metric (10,000 Operations)     | Splay Tree        | AVL Tree          |
  +---------------------------------------+-------------------+-------------------+
  | Skewed (80/5) Avg Nodes Visited       | 8.86              | 9.04              |
  | Skewed Worst Single Access            | 959               | 10                |
  | Uniform Avg Nodes Visited             | 12.53             | 8.98              |
  | Sequential Access Avg Nodes (1..1000) | 3.30              | 9.97 (log2 N)     |
  +---------------------------------------+-------------------+-------------------+
  ```

---

### Question 6: Trie (Phone Contact Search & Autocomplete)
- **Files**:
  - `q6_trie/complete_trie.c`: Completed phone contact search program.
  - `q6_trie/trie.c`: Full Trie supporting insert, exact search, prefix count, alphabetical autocomplete DFS, word deletion, and scalability benchmarks for 500, 5,000, and 50,000 contacts comparing memory against a contiguous sorted array.
  - `q6_trie/analysis.md`: $O(m)$ search proof, memory calculations ($26 \times 8$ bytes/node), radix compression, and binary search alternative.
- **Sample Output**:
  ```text
  a:5 an:4 anu:2 b:1 z:0 
  
  +---------------------------------------------------------------+
  | Dataset: 50,000 Contact Names                                 |
  +---------------------------------------------------------------+
  | Total Trie Nodes Created         | 239091                     |
  | Trie Memory Consumed             | 50433.26 KB (49.25 MB)     |
  | Sorted Array Memory Consumed     | 805.23   KB ( 0.79 MB)     |
  | Memory Ratio (Trie / Array)      | 62.6x                      |
  | Average Latency Per Prefix Search| 0.007 us (sub-microsecond!)|
  +---------------------------------------------------------------+
  ```

---

### Question 7: Regional Language App Dictionary (200,000 Words)
- **Files**:
  - `q7_dictionary/dictionary.c`: Ternary Search Tree (TST) + Contiguous Sorted Index engine supporting exact look-up ($O(m)$), keystroke autocomplete ($O(m + k)$), cache-optimal alphabetical range listing ($O(\log N + k)$), and Damerau-Levenshtein edit-distance-1 spell-checking.
  - `q7_dictionary/design.md`: 4-stage engineering design, measurable criteria, comparison matrix, and Hash Table + Sorted Array analysis.
  - `q7_dictionary/analysis.md`: Scaling analysis for 2,000,000 words (DAFSA / LOUDS) and Unicode combining character limitations.
- **Sample Output**:
  ```text
  Successfully built 1,000-word Dictionary Engine in 0.30 ms
  Total TST Nodes Allocated: 1119 | Resident RAM: 61.29 KB (0.06 MB)
  Projected RAM for Full 200,000 Words: ~11.97 MB (Within 25 MB mobile ceiling!)
  
  --- TEST 1: EXACT LOOK-UP ---
  Look-up 'acaru6': EXISTS (latency: 0.0004 ms)
  
  --- TEST 2: AUTOCOMPLETE ---
  Autocomplete suggestions for prefix 'ka': kaai1, kaai2, kaai3, kaai4, kaai5
  
  --- TEST 3: ALPHABETICAL RANGE LISTING ---
  Alphabetical Range Listing between 'be' and 'ca': Total words in range: 80
  
  --- TEST 4: SPELL-CHECK SUGGESTIONS (EDIT DISTANCE 1) ---
  Spell-Check Suggestions for 'caaru6':
      Suggestion (Transposition): acaru6
      Suggestion (Substitution):  kaaru6
      Suggestion (Substitution):  maaru6
  ```

---

## 4. Screenshot Evidence Gallery

High-resolution console screenshots rendered with Python (`matplotlib` / `Pillow`) are saved in `screenshots_placeholder/` and `output/screenshots/`:

| Question | Generated Screenshot File | Link |
| :--- | :--- | :--- |
| **Q1: BST** | `01_q1_bst_terminal.png` | [View Screenshot](file:///D:/CSA0307-DS/AT3/screenshots_placeholder/01_q1_bst_terminal.png) |
| **Q2: AVL** | `02_q2_avl_terminal.png` | [View Screenshot](file:///D:/CSA0307-DS/AT3/screenshots_placeholder/02_q2_avl_terminal.png) |
| **Q3: B-Tree** | `03_q3_btree_terminal.png` | [View Screenshot](file:///D:/CSA0307-DS/AT3/screenshots_placeholder/03_q3_btree_terminal.png) |
| **Q4: Red-Black** | `04_q4_rbtree_terminal.png` | [View Screenshot](file:///D:/CSA0307-DS/AT3/screenshots_placeholder/04_q4_rbtree_terminal.png) |
| **Q5: Splay** | `05_q5_splay_terminal.png` | [View Screenshot](file:///D:/CSA0307-DS/AT3/screenshots_placeholder/05_q5_splay_terminal.png) |
| **Q6: Trie** | `06_q6_trie_terminal.png` | [View Screenshot](file:///D:/CSA0307-DS/AT3/screenshots_placeholder/06_q6_trie_terminal.png) |
| **Q7: Dictionary** | `07_q7_dictionary_terminal.png` | [View Screenshot](file:///D:/CSA0307-DS/AT3/screenshots_placeholder/07_q7_dictionary_terminal.png) |

---

## 5. Git Commands for GitHub Submission

To push this entire project to your personal GitHub repository:

```bash
# 1. Initialize git repository (if not already initialized)
git init

# 2. Stage all source files, documentation, outputs, and screenshots
git add .

# 3. Create initial submission commit
git commit -m "Complete Assignment 3 (CO3) Tree-Based Structures with all tasks, benchmarks, and screenshots"

# 4. Set default branch to main
git branch -M main

# 5. Link your GitHub remote repository (replace with your personal repo URL)
git remote add origin https://github.com/<YOUR_GITHUB_USERNAME>/<YOUR_REPOSITORY_NAME>.git

# 6. Push all commits and branches to GitHub
git push -u origin main
```
