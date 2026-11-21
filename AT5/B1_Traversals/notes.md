# Task B1 Notes — Traversals, Representations & Network Resilience

## 1. What the Programs Show
- **`b1_bfs.c`**: Implements BFS on the 7-station Chennai Metro network stored in Compressed Sparse Row (CSR) format:
  - Traversal order: `Airport Guindy Saidapet Vadapalani Central Koyambedu Egmore`.
  - Shortest path to Koyambedu: `Koyambedu is 3 stops away via: Koyambedu Vadapalani Guindy Airport`.
- **`b1_queue_trace.c`**: Traces the FIFO queue dynamics at every step, confirming breadth levels (hop distance 0, 1, 2, 3).
- **`b1_components_cycles.c`**:
  - Implements connectivity analysis on the 7-node metro and 20-node Chennai road network.
  - Flooding simulation severing river bridges (Adyar/Cooum rivers) fragments the city from 1 connected component into multiple disconnected islands.
  - Cycle detection using DFS with parent back-edge checking accurately identifies arterial loops in the road network while verifying cycle-free structure on feeder trees.
- **`b1_matrix_vs_list.c`**: Measures concrete scaling trade-offs between dense adjacency matrices and CSR adjacency lists.
- **`alt_dijkstra_unit.c`**: Proves that while binary-heap Dijkstra correctly finds fewest-stops on unweighted graphs, simple FIFO BFS is $\sim 3.3\times$ faster and requires zero priority-queue heap overhead.

## 2. Empirical Benchmark Measurements (Median of 3 Runs)
Comparing Adjacency Matrix vs CSR Adjacency List for sparse city networks ($E \approx 3V$):

| Vertices ($V$) | Edges ($E$) | Matrix BFS Time | CSR List BFS Time | Speedup Factor | Matrix Memory | CSR List Memory | Memory Reduction |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **1,000** | 3,000 | 0.000397 s | 0.000013 s | **30.5x** | 0.95 MB | 15.66 KB | **60.6x less memory** |
| **10,000** | 30,000 | 0.037481 s | 0.000242 s | **154.9x** | 95.37 MB | 156.29 KB | **610.2x less memory** |
| **50,000** | 150,000 | 1.034657 s | 0.001446 s | **715.5x** | 2.33 GB | 781.29 KB | **3,055x less memory** |

### Fewest Stops Comparison (BFS vs Unit-Weight Dijkstra):
- At $V = 10{,}000$: BFS takes **0.000239 s** vs Dijkstra **0.000758 s** (Dijkstra is $3.17\times$ slower, uses +312 KB heap).
- At $V = 50{,}000$: BFS takes **0.001223 s** vs Dijkstra **0.004079 s** (Dijkstra is $3.33\times$ slower, uses +1562 KB heap).

### Engineering Conclusion:
For city road networks, vertex degree is naturally bounded by street topology ($d \le 4\text{ to }6$). Adjacency matrices incur $O(V^2)$ storage and scan billions of non-existent zero entries during traversals. CSR adjacency lists reduce memory from gigabytes to kilobytes while delivering over $700\times$ faster traversals.
