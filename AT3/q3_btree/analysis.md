# Question 3: Design & Analysis, Conclusion, and Alternative Concepts

## 1. Design & Analysis Answers

### Item 29: Why B-Trees are Wide and Short
- **Branching Factor**: While binary trees have a branching factor of 2, a B-tree of order $M$ can have up to $M$ children (and $M-1$ keys) per node. For example, with $M = 100$, each node fans out up to 100 ways.
- **Tree Height Minimization**:
  The height of a B-tree scales as $\approx \log_M N$ instead of $\log_2 N$.
  For $N = 5,000,000$ keys:
  $$\text{Binary Tree Height} \approx \log_2(5,000,000) \approx 23$$
  $$\text{B-Tree (Order 100) Height} \le \log_{50}\left(\frac{5,000,000 + 1}{2}\right) \approx 3.76 \implies 4 \text{ levels!}$$
- Since reading from mechanical disk or SSD incurs a high latency penalty per block access ($\approx 10\text{ ms}$ for HDD, $\approx 0.1\text{ ms}$ for SSD), minimizing the number of disk accesses is the primary performance objective. A short, wide tree minimizes block transfers.

---

### Item 30: Node Splitting and Why All Leaves Stay at the Same Depth
- In a BST or AVL tree, new nodes are inserted as leaves at the bottom, causing different branches to grow downward at different rates.
- In contrast, a **B-tree grows upwards, not downwards**:
  1. New keys are always initially inserted into an existing **leaf node**.
  2. If a leaf node exceeds its capacity ($2t - 1$ keys), it is **split**: the median key moves up to the parent node, and the remaining keys form two half-sized siblings.
  3. If the parent is also full, the split cascades upwards.
  4. Only when the **root itself** splits is a new root created containing the promoted median key and two child pointers. This increases the depth of *every* leaf node by exactly 1 simultaneously.
- **Invariant**: Because splitting pushes keys up towards the parent and increases height only by adding a new root above the existing tree, **all leaves remain at the exact same depth** at all times.

---

### Item 31: B-Tree vs B+ Tree for Range Queries
*Context: Finding all hospital patients admitted within a given calendar week.*

| Feature | Standard B-Tree | B+ Tree |
| :--- | :--- | :--- |
| **Data Storage** | Keys and data records reside in both internal nodes and leaf nodes. | Internal nodes contain **keys only** (routing indices); all records reside strictly in **leaves**. |
| **Leaf Structure** | Isolated leaf nodes without horizontal links. | All leaf nodes are linked in a **doubly linked list**. |
| **Range Query Mechanism** | Must perform an **inorder traversal** jumping back and forth across internal nodes and leaves. Each step requires random disk I/O. | Perform **one binary search** to find the start of the range, then perform a **contiguous linear scan** along the leaf linked list. |
| **Disk I/O Efficiency** | Poor for ranges; causes multiple random seeks up and down the tree. | **Optimal**: sequential block reads with high prefetching efficiency. |
| **Internal Node Capacity**| Smaller branching factor (space taken by record payloads). | **Higher branching factor** (internal nodes store only keys/pointers), making the tree even shallower. |

---

### Item 32: Choosing Node Size to Match a Disk Block
- Physical secondary storage (HDDs, SSDs, OS file systems) does not read single bytes from disk; it reads data in fixed-size **disk blocks / pages** (typically $4\text{ KB} = 4,096\text{ bytes}$ or $8\text{ KB}$).
- If a node is smaller than 4 KB, reading that node transfers a full 4 KB block anyway, wasting I/O bandwidth.
- If a node is larger than 4 KB, reading it requires multiple disk sector reads.
- **Formula for Choosing Order $M$**:
  Let $B$ = Disk Block Size (e.g., 4096 bytes), $P$ = Child Pointer Size (8 bytes on 64-bit), $K$ = Key Size (8 bytes), and $R$ = Record Pointer Size (8 bytes).
  A node with $M$ children contains $(M-1)$ keys and $(M-1)$ record pointers:
  $$(M - 1) \cdot K + (M - 1) \cdot R + M \cdot P \le B$$
  $$(M - 1)(8 + 8) + 8M \le 4096 \implies 16M - 16 + 8M \le 4096 \implies 24M \le 4112 \implies M \approx 171$$
  Choosing $M \approx 100 - 200$ perfectly fills exactly one physical disk page per node read!

---

## 2. Conclusion: B-Tree for Disk vs AVL for In-Memory

- **Disk / Databases (Why B-Tree)**:
  - Reading a byte from disk costs $\approx 10\text{ ms}$, whereas CPU instructions take $\approx 1\text{ ns}$ ($10,000,000\times$ difference!).
  - B-trees minimize the number of block reads to $\le 3-4$ reads per query.
  - The CPU cost of doing a binary search across 100 keys inside a loaded block in RAM ($\approx 10\text{ ns}$) is completely negligible compared to the 10 ms disk transfer.
- **In-Memory (Why AVL)**:
  - In RAM, there is no disk block boundary; all memory accesses take $\approx 10-50\text{ ns}$.
  - An AVL tree requires fewer comparisons overall ($O(\log_2 n)$ binary comparisons) and simpler pointer operations.
  - In a B-tree in RAM, inserting into a node requires shifting keys within the array, which creates unnecessary memory copy overhead.

---

## 3. Alternative Concept and Complexity Analysis: AVL Tree on Disk with One Node Per Block

If the patient index were stored on disk using an **AVL Tree** where each node occupies one disk block:

1. **Number of Disk Reads**:
   - For $N = 5,000,000$ patient records, the AVL height is $\approx 1.44 \log_2(5,000,000) \approx 32-33$.
   - A search requires traversing from root to leaf, incurring **$23$ to $33$ random disk reads**.
   - At $10\text{ ms}$ per read:
     $$\text{Search Time} = 28 \times 10\text{ ms} = \mathbf{280\text{ ms}} \quad (\text{vs } 4 \times 10\text{ ms} = \mathbf{40\text{ ms}} \text{ in B-Tree})$$
   - This represents a **$7\times$ slowdown** per search query!
2. **Space Wasted in Disk Blocks**:
   - Each disk block is $4,096\text{ bytes}$.
   - An AVL node contains: `key` (8 bytes) + `height` (4 bytes) + `left` (8 bytes) + `right` (8 bytes) = **28 bytes**.
   - Wasted space per block:
     $$4,096 - 28 = \mathbf{4,068\text{ bytes wasted per block}} \implies \mathbf{99.3\% \text{ storage waste!}}$$
   - For 5 million records, storing 1 AVL node per block consumes $5,000,000 \times 4\text{ KB} \approx \mathbf{20\text{ GB}}$ of disk space, whereas a B-tree packs $\approx 100$ records per block, consuming only $\approx \mathbf{200\text{ MB}}$ ($100\times$ less disk space!).
