# Consolidated Experimental Results & Benchmark Measurements

This document aggregates the empirical measurements, latency profiles, tree heights, and memory footprints collected across all seven tree data structures under C11 (`gcc -Wall -Wextra -std=c11 -g -O2`) on a 64-bit platform using `clock_gettime(CLOCK_MONOTONIC)`.

---

## 1. Summary Benchmark Matrix (Q1 to Q7)

| Question / Structure | Primary Problem Context | Dataset Scale ($N$) | Tree Height | Search Comparisons / Node Visits | Search Latency / Query | Insert / Build Latency | Total Memory Footprint |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Q1: Plain BST (Random)** | Library Catalogue | $2,000$ books | $26$ | $14.18$ avg | $0.084\text{ \mu s}$ | $0.261\text{ ms}$ | $48.0\text{ KB}$ |
| **Q1: Plain BST (Sorted)** | Library Catalogue | $2,000$ books | $1999$ (degenerate) | $1,000.50$ avg | $2.57\text{ \mu s}$ ($30\times$ slower) | $18.15\text{ ms}$ ($70\times$ slower) | $48.0\text{ KB}$ |
| **Q2: AVL Tree (Sorted)** | Stock Price Index | $100,000$ quotes | **$17$** | $\approx 16.5$ | **$0.102\text{ \mu s}$** | $18.31\text{ ms}$ ($1,880\times$ faster)| $3.20\text{ MB}$ |
| **Q2: Plain BST (Sorted)** | Stock Price Index | $100,000$ quotes | **$99,999$** | $50,000.0$ | **$469.9\text{ \mu s}$** ($4,580\times$ slower)| $34,434.0\text{ ms}$ | $2.40\text{ MB}$ |
| **Q3: B-Tree ($M=100, t=50$)**| Patient Database on Disk | $5,000,000$ patients | **$4$ levels** | **$4$ node reads** | **$40.0\text{ ms}$** (@10ms disk seek) | N/A (proactive split) | $\approx 200\text{ MB}$ on disk |
| **Q3: AVL Tree on Disk** | Patient Database on Disk | $5,000,000$ patients | **$33$ levels** | **$33$ node reads** | **$330.0\text{ ms}$** ($8.2\times$ slower) | $O(\log n)$ disk writes | $\approx 20\text{ GB}$ (99% waste) |
| **Q4: Red-Black Tree** | Linux CFS Scheduler | $10,000$ processes | **$16$** (theory $\le 26$) | $\le 16$ | **$O(1)$ cached leftmost** | $5,855$ rotations (0.58/ins)| $320\text{ KB}$ |
| **Q5: Splay Tree (Skewed 80/5)**| E-Commerce Product Cache | $1,000$ products (10k ops)| Dynamic root | **$8.86$ avg** (improves) | Near $O(1)$ for top 5% | Zero balance metadata | $16.0\text{ KB}$ |
| **Q5: AVL Tree (Skewed 80/5)** | E-Commerce Product Cache | $1,000$ products (10k ops)| $10$ (fixed) | **$9.04$ avg** | Rigid $O(\log n)$ | Height stored in node | $24.0\text{ KB}$ |
| **Q6: 26-Way Trie** | Mobile Contact Search | $50,000$ contacts | N/A ($m \le 10$) | $m$ characters ($O(m)$) | **$0.007\text{ \mu s}$** (instant) | $O(m)$ | $49.25\text{ MB}$ (high) |
| **Q6: Contiguous Sorted Array**| Mobile Contact Search | $50,000$ contacts | N/A | $O(m \log n)$ binary | $1.20\text{ \mu s}$ | $O(n)$ shifting | **$0.79\text{ MB}$** ($62\times$ leaner)|
| **Q7: TST + Sorted Index** | Regional App Dictionary | $200,000$ words (projected)| Balanced TST | $O(m + \log \Sigma)$ | **$0.001\text{ ms}$** (lookup / auto) | Background batch merge | **$\approx 11.97\text{ MB}$** (< 25MB) |

---

## 2. Key Observations & Takeaways

1. **Self-Balancing Necessity (Q1 vs Q2)**:
   When inputs arrive in sequential order, a plain BST degenerates into a single-linked list of height $n - 1$, causing insertion to collapse from $O(\log n)$ to $O(n^2)$ and search from $O(\log n)$ to $O(n)$. An AVL tree guarantees strict logarithmic height ($h \le 1.44 \log_2 n$) with minimal rotation overhead.
2. **Disk Block Matching (Q3)**:
   In secondary storage (HDD/SSD), physical block seek latency ($10\text{ ms}$) dwarfs in-memory CPU computation. A B-Tree's high branching factor ($M \ge 100$) compresses 5 million records into just 4 disk reads, reducing query latency from $330\text{ ms}$ to $40\text{ ms}$ while eliminating $99\%$ of wasted disk block capacity.
3. **Write-Heavy Scheduling (Q4)**:
   Red-Black trees limit insertion rotations to at most $2$ and deletion rotations to at most $3$, outperforming AVL trees where deletion can cascade up to $O(\log n)$ structural rotations. Adding a cached leftmost pointer enables $O(1)$ task dispatching as used in the Linux kernel Completely Fair Scheduler.
4. **Adaptive Caching (Q5)**:
   Splay trees require zero balance bytes per node. By moving accessed keys to the root via Zig, Zig-Zig, and Zig-Zag operations, frequently viewed items are located in $O(1)$ to $O(3)$ node visits under 80/5 Pareto workloads.
5. **String Matching & Mobile Footprint (Q6 & Q7)**:
   While standard 26-way Tries deliver sub-microsecond prefix searches, their pointer density ($26 \times 8 = 208$ bytes per node) is memory-prohibitive on mobile devices ($49\text{ MB}$ for 50,000 contacts). A Ternary Search Tree (TST) paired with a contiguous sorted array achieves sub-millisecond autocomplete while compressing a 200,000-word dictionary into just $12\text{ MB}$ of resident RAM.
