# Task A5 Notes — Prim's MST, Shortest Paths vs MST, and Kruskal Scaling

## 1. What the Programs Show
- **`a5_prim.c`**: Implements Prim's greedy algorithm ($O(V^2)$) on a 5-village fibre-optic cable network.
  - Starting from village 0, repeatedly attaches the minimum-cost edge connecting an unvisited vertex to the growing tree.
  - Formed MST edges: `0-1(2) 1-2(3) 0-3(6) 1-4(5)` with exact **total cost = 16 lakh**.
- **`a5_mst_vs_sp.c`**: Evaluates whether an MST yields shortest paths between arbitrary pairs of vertices:
  - **Shortest Path (Dijkstra)** between Node 4 and Node 2: Route is `4 -> 2` with direct cost **7 lakh**.
  - **Path inside the MST** between Node 4 and Node 2: Route is `4 -> 1 -> 2` with cost $5 + 3 = \mathbf{8\text{ lakh}}$.
  - **Core Engineering Takeaway**: A Minimum Spanning Tree minimizes the **total infrastructure cost** to connect all nodes together, but does **not** provide optimal individual point-to-point journey times.
- **`alt_kruskal.c`**: Implements Kruskal's algorithm using Disjoint Set Union (DSU) with path compression and union-by-rank. Confirms exact total cost of 16 lakh matching Prim.

## 2. Empirical Benchmark Measurements (Median of 3 Runs)
Fixed random seed `srand(42)`:

| Graph Characteristic | Vertices ($V$) | Undirected Edges ($E$) | Prim (Array) $O(V^2)$ | Prim (Binary Heap) $O(E \log V)$ | Kruskal (DSU) $O(E \log E)$ | Winning Algorithm |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Sparse Network** | 10,000 | 30,000 | 0.070780 s | 0.002792 s | **0.002418 s** | **Kruskal / Heap Prim** (~29x faster) |
| **Dense Network** | 2,000 | 1,000,000 | **0.004014 s** | 0.009265 s | 0.073163 s | **Prim (Array)** (~18x faster than Kruskal) |

### Algorithmic Decisions:
- **Sparse Graphs ($E \approx 3V$)**: Kruskal and Heap-Prim avoid scanning non-existent edges. Kruskal sorting $30{,}000$ edges takes mere milliseconds.
- **Dense Graphs ($E \approx V^2 / 4$)**: Array Prim dominates because $O(V^2)$ scanning on array keys incurs zero heap rebalancing and avoids sorting $1{,}000{,}000$ edges in Kruskal.
