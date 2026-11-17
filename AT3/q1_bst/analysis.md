# Question 1: Design & Analysis, Conclusion, and Alternative Concepts

## 1. Design & Analysis Answers

### Item 5: Why Inorder Traversal of a BST Gives Sorted Order
A Binary Search Tree (BST) enforces the **BST Invariant** at every node $x$:
$$\forall k_L \in \text{LeftSubtree}(x), \; k_L < x.\text{key} \quad \text{and} \quad \forall k_R \in \text{RightSubtree}(x), \; k_R > x.\text{key}$$

Inorder traversal recursively visits:
1. All nodes in $\text{LeftSubtree}(x)$ (recursively in order),
2. The current node $x$,
3. All nodes in $\text{RightSubtree}(x)$ (recursively in order).

By mathematical induction on tree height:
- **Base Case**: A tree with 0 or 1 node is trivially sorted.
- **Inductive Step**: Assuming the left subtree outputs its elements in sorted order $S_L$ and the right subtree outputs in sorted order $S_R$. Because all elements in $S_L < x.\text{key}$ and $x.\text{key} < \text{all elements in } S_R$, the concatenated sequence $[S_L, \; x.\text{key}, \; S_R]$ is strictly ascending. Thus, inorder traversal always yields keys in monotonically increasing sorted order.

---

### Item 6: Three Cases of Deletion and Use of Inorder Successor
When deleting a key $k$ from node $t$:
1. **Case 1: Leaf Node (0 children)**:
   - Neither left nor right child exists.
   - Simply unlink $t$ from its parent and `free(t)`. No subtrees are disturbed.
2. **Case 2: Node with One Child**:
   - Exactly one child exists (either left or right).
   - Splice out node $t$ by connecting $t$'s parent directly to $t$'s single child, then `free(t)`. The BST invariant is preserved because the single child already satisfied the parent's directional bound.
3. **Case 3: Node with Two Children**:
   - Both children exist; removing $t$ would leave two orphaned subtrees.
   - We replace $t$'s key with its **inorder successor** (the minimum node in $t$'s right subtree, found by moving right once and then as far left as possible).
   - **Why Inorder Successor?**
     - The successor is strictly greater than all nodes in $t$'s left subtree and smaller than or equal to all other nodes in $t$'s right subtree.
     - By replacing $t$'s key with the successor's key, the BST invariant is preserved at node $t$.
     - Crucially, the successor node has at most one child (it cannot have a left child by definition of being leftmost in the right subtree). Thus, deleting the original successor node reduces to Case 1 or Case 2, which is straightforward.

---

### Item 7: Best- and Worst-Case Height of a BST with $n$ Nodes
The height $h$ is defined as the maximum number of edges from the root to any leaf (empty tree $= -1$, single node $= 0$).

1. **Best Case (Completely Balanced Tree)**:
   - At each level $i$ (from $0$ to $h$), the level has $2^i$ nodes.
   - Total nodes:
     $$n = \sum_{i=0}^h 2^i = 2^{h+1} - 1 \implies 2^{h+1} = n + 1 \implies h = \lceil \log_2(n+1) \rceil - 1 = \lfloor \log_2 n \rfloor$$
   - Thus, best-case height is $\Theta(\log_2 n)$.

2. **Worst Case (Degenerate / Skewed Chain)**:
   - Every node has only one child (either all right or all left), forming a linked list.
   - For $n$ nodes, there is 1 node per level from level $0$ to $n-1$.
   - Thus, worst-case height is $h = n - 1 = \Theta(n)$.

---

### Item 8: Why Sorted Input Produces a Degenerate Tree
When keys are inserted in strictly increasing sorted order (e.g., $10, 20, 30, \dots$):
- Key 1 is the root.
- Key 2 is greater than Key 1 $\implies$ placed as the right child of Key 1.
- Key 3 is greater than Key 1 and Key 2 $\implies$ placed as the right child of Key 2.
- In general, key $i$ is greater than all previously inserted keys $\{1, \dots, i-1\}$. It traverses all existing right pointers and attaches as the right child of key $i-1$.
- No node ever receives a left child. The tree degenerates into a right-skewed linear linked list of height $n - 1$.
- Consequently, search and insertion time degrades from $O(\log n)$ to $O(n)$, identical to linear search in an unsorted list.

---

### Complete the Code Question: Why 8 Keys Give Height 3 vs Height 7
- **Random Array** `{500, 300, 700, 200, 400, 600, 800, 350}`:
  - 500 splits the keys into smaller ($\le 400$) and larger ($\ge 600$).
  - 300 further splits $\{200\}$ and $\{400, 350\}$.
  - 700 splits $\{600\}$ and $\{800\}$.
  - The tree branches symmetrically on both sides, yielding 4 levels (edges $0, 1, 2, 3$), so **height = 3**.
  - Accessing key 350 traverses `500 -> 300 -> 400 -> 350`, taking **4 comparisons**.
- **Sorted Array** `{10, 20, 30, 40, 50, 60, 70, 80}`:
  - Each key is strictly larger than the last. Each key attaches as the right child of the previous key.
  - The tree is a single straight chain of 8 nodes with 7 edges, so **height = 7**.
  - Accessing key 80 requires visiting all 8 nodes from root 10 to leaf 80, taking **8 comparisons**.

---

## 2. Conclusion: Plain BST vs Self-Balancing Tree

- **When a Plain BST is Sufficient**:
  1. **Randomized or Pre-Shuffled Data**: When the arrival order of keys is uniformly random, the expected height of a BST is approximately $2 \ln n \approx 1.39 \log_2 n = O(\log n)$.
  2. **Read-Mostly Static Workloads**: When the tree is built once by inserting the median element first (or sorting and building recursively), providing a perfectly balanced tree without balancing overhead.
  3. **Low-Memory Constraints**: A plain BST requires only 2 pointers per node with zero balancing metadata (no height, balance factor, or red-black bit), saving memory.

- **When It Must Be Replaced by a Self-Balancing Tree (AVL / Red-Black)**:
  1. **Ordered or Clustered Inputs**: Whenever inputs can arrive presorted, reverse-sorted, or clustered (e.g., auto-incrementing database primary keys, accession numbers, timestamp logs), a plain BST degenerates to $O(n)$.
  2. **Real-Time Latency Guarantees**: Interactive applications cannot tolerate $O(n)$ worst-case stalls. Self-balancing trees guarantee strict $O(\log n)$ time for search, insertion, and deletion.

---

## 3. Alternative Concept and Complexity Analysis: Sorted Array with Binary Search

If the library catalogue were implemented as a **Sorted Array** with **Binary Search** instead of a BST:

| Operation / Property | Plain BST (Average / Worst) | Balanced BST (AVL/RB) | Sorted Array + Binary Search |
| :--- | :--- | :--- | :--- |
| **Search Time** | $O(\log n)$ avg / $O(n)$ worst | $O(\log n)$ guaranteed | $O(\log n)$ guaranteed |
| **Insertion Time** | $O(\log n)$ avg / $O(n)$ worst | $O(\log n)$ | $O(n)$ (requires shifting elements) |
| **Deletion Time** | $O(\log n)$ avg / $O(n)$ worst | $O(\log n)$ | $O(n)$ (requires shifting elements) |
| **Sorted Listing** | $O(n)$ (inorder traversal) | $O(n)$ (inorder traversal) | $O(n)$ (contiguous linear array scan) |
| **Pointer Overhead** | 2 pointers per node ($16$ bytes on 64-bit) | 2 pointers + metadata ($16-24$ bytes) | **0 bytes** (contiguous payload only) |
| **Cache Locality** | Poor (nodes scattered across heap) | Poor (heap nodes) | **Optimal** (contiguous cache lines preloaded) |

### Detailed Trade-Off Summary
1. **Search**: Both achieve $O(\log n)$ search time, but the sorted array has significantly lower constant factors due to CPU cache line prefetching.
2. **Insertion**: In a sorted array, inserting a new accession number requires finding the index ($O(\log n)$) and then shifting on average $n/2$ elements to the right ($O(n)$). In a BST, insertion only updates child pointers ($O(h)$), without moving existing nodes in memory.
3. **Sorted Listing**: Both output elements in $O(n)$, but the sorted array requires zero recursion or stack space and reads sequentially from RAM.
4. **Memory Footprint**: A sorted array has zero pointer overhead. A BST requires 16 bytes of pointer overhead per record on a 64-bit architecture, plus dynamic memory allocator header overhead per node (typically 8–16 bytes per `malloc`).
