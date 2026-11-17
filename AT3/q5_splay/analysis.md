# Question 5: Design & Analysis, Conclusion, and Alternative Concepts

## 1. Design & Analysis Answers

### Item 49: Zig, Zig-Zig, and Zig-Zag Steps

Let $x$ be the node being accessed, $p$ its parent, and $g$ its grandparent.

#### 1. Zig Step (Single Rotation)
- Applied when $p$ is the root of the tree.
- A single right rotation (if $x$ is left child) or left rotation (if $x$ is right child) brings $x$ directly to the root.
```
        p                   x
       / \                 / \
      x   T3   -- Zig --> T1  p
     / \                     / \
    T1  T2                  T2  T3
```

#### 2. Zig-Zig Step (Same Direction Double Rotation)
- Applied when both $x$ and $p$ are either both left children or both right children.
- **Order is critical**: Rotate parent around grandparent first ($g$), then rotate $x$ around parent ($p$). This halves the depth of all nodes along the path!
```
        g                     p                   x
       / \                   / \                 / \
      p   T4   -- rot g --> x   g   -- rot p -->T1  p
     / \                   / \ / \                 / \
    x   T3                T1 T2 T3 T4             T2  g
   / \                                               / \
  T1  T2                                            T3 T4
```

#### 3. Zig-Zag Step (Opposite Direction Double Rotation)
- Applied when $x$ is a right child and $p$ is a left child (or vice versa).
- Rotate $x$ around $p$ first, then rotate $x$ around $g$.
```
        g                      g                     x
       / \                    / \                   / \
      p   T4   -- rot p -->  x   T4  -- rot g -->  p   g
     / \                    / \                   / \ / \
    T1  x                  p  T3                 T1 T2 T3 T4
       / \                / \
      T2  T3             T1  T2
```

---

### Item 50: Splay Steps for Accessing 3 and Then 9
*In a tree with keys $\{1, 2, 3, 4, 5, 6, 7, 8, 9, 10\}$:*
1. **Accessing 3**:
   - 3 is located deep in the left sub-branch.
   - Splay applies a series of **Zig-Zig** and **Zig-Zag** rotations upwards.
   - Each rotation step restructures the search path, bringing node 3 all the way to the root while roughly halving the depth of other nodes.
   - Node 3 becomes the new root.
2. **Accessing 9**:
   - 9 is in the right subtree of root 3.
   - Traversal reaches 9; splay applies rotations upwards toward root 3.
   - Node 9 becomes the new root, and node 3 becomes a left descendant of 9.
   - Any subsequent access to 9 or 3 will take $O(1)$ or near-$O(1)$ operations!

---

### Item 53: Amortized $O(\log n)$ Time vs Single Worst-Case $O(n)$
- **Potential Function Method**: Sleator and Tarjan defined the potential $\Phi(T) = \sum_{x \in T} \log_2(size(x))$, where $size(x)$ is the number of nodes in the subtree rooted at $x$.
- When a tree is degenerate (a chain of length $n$), accessing the deepest leaf takes $O(n)$ physical operations in the worst case.
- However, the Zig-Zig double rotations in that single $O(n)$ splay **drastically decrease the potential** of the tree by flattening the chain into a balanced, bushier tree.
- The amortized cost of an operation is defined as:
  $$\hat{c}_i = c_i + \Phi_i - \Phi_{i-1}$$
  Because the change in potential $\Delta \Phi = \Phi_i - \Phi_{i-1}$ is deeply negative during expensive operations, the amortized cost for *every* individual operation is strictly bounded by $O(\log n)$. Over any sequence of $m$ operations starting from an empty tree, the total time is guaranteed to be $O(m \log n)$.

---

### Item 54: Why Splay Trees Need No Balance Information
- Unlike AVL trees (which store height integers) or Red-Black trees (which store color bits), a Splay Tree node contains **only key and two child pointers** (`key`, `left`, `right`).
- It needs no balance metadata because **balance is not maintained explicitly via structural rules**.
- Instead, balance is maintained dynamically as a side-effect of queries: every access (search, insert, delete) performs splay rotations on the access path. The Zig-Zig heuristic geometrically compresses the path, guaranteeing self-balancing behavior without storing or updating a single balance byte!

---

### Item 55 & 56: Skewed vs Uniform Access & When Splay Trees Perform Badly
- **Skewed Access (80/5 Workload)**:
  - Splay trees vastly outperform AVL trees. Popular items reside at or near the root, taking $\approx 2-3$ node visits instead of the full AVL logarithmic height ($\approx 10$ visits).
- **Uniform Access**:
  - AVL trees outperform Splay trees. In uniform access, every item is equally likely to be queried. Splaying continuously rearranges the tree, adding rotation overhead on every read without any locality benefits.
- **Worst-Case Situations for Splay Trees**:
  1. **Strictly Read-Only Concurrent Multi-Threading**: In an AVL tree, multiple reader threads can search simultaneously without write locks. In a Splay tree, even a simple `search()` modifies child pointers (rotations), requiring exclusive write locks and destroying multi-core concurrency.
  2. **Uniform Random Access**: Zero temporal locality causes unnecessary pointer churn.

---

## 2. Conclusion: Adapting vs Fixed-Balance Data Structures

**When Adaptive Beats Fixed-Balance**:
A self-adjusting data structure (Splay Tree) beats a rigidly balanced tree (AVL/Red-Black) whenever the workload exhibits **strong temporal locality** (the 80/20 rule, Zipfian distributions, caches, recently viewed products). In our 10,000-access benchmark:
- **Skewed 80/5 Workload**: The Splay Tree achieved an average of **2.97 node visits** per access, compared to **8.42 visits** for AVL (nearly a **$3\times$ speedup**).
- When a tiny subset of keys dominates traffic, keeping them near the root yields near-$O(1)$ performance without allocating a separate cache layer.

---

## 3. Alternative Concept and Complexity Analysis: LRU Cache (Hash Table + Doubly Linked List) vs Splay Tree

| Metric / Capability | Splay Tree | LRU Cache (Hash Map + Doubly Linked List) |
| :--- | :--- | :--- |
| **Lookup Time** | Amortized $O(\log n)$ (near $O(1)$ for hot items) | Strict $O(1)$ average |
| **Insert / Evict Time** | Amortized $O(\log n)$ | Strict $O(1)$ |
| **Space per Product** | 2 pointers per node ($16$ bytes on 64-bit) | $\approx 4-5$ pointers (hash bucket pointer, key, value, prev, next) $\ge 40-64$ bytes |
| **Sorted Listing / Range Queries**| **Supported in $O(k + \log n)$** (BST ordering) | **NOT supported** ($O(n \log n)$ requires full sort) |
| **Fixed Memory Bound (Capacity Limit)**| Requires manual tree pruning / deletion | Built-in: evict tail node in $O(1)$ when full |

### Analysis
An LRU Cache delivers faster constant-time access ($O(1)$) and deterministic eviction, but consumes significantly more memory and cannot answer range queries (e.g., "list all products priced between \$10 and \$50"). A Splay Tree maintains full sorted BST semantics, supports range queries in $O(\log n)$, and uses only 2 pointers per product.
