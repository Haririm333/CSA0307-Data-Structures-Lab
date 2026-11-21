# Task A4 Notes — Floyd-Warshall All-Pairs Shortest Paths & Scaling

## 1. What the Programs Show
- **`a4_floyd.c`**: Demonstrates the classic $O(V^3)$ triple-nested dynamic programming algorithm for all-pairs shortest paths on a 4-vertex road network.
  - Initial matrix contains direct road costs (`999` denotes absence of direct road).
  - Final distance matrix:
    ```
       0   5   8   9
     999   0   3   4
     999 999   0   1
     999 999 999   0
    ```
- **`a4_floyd_steps.c`**: Traces the transition of matrix $D^{(k)}$ as intermediate vertices $k \in \{0, 1, 2, 3\}$ are incrementally allowed:
  - **At $k=1$**: Relaxing through vertex 1 finds path $0 \to 1 \to 2$ with cost $5 + 3 = 8$, reducing $d[0][2]$ from $\infty$ to 8.
  - **At $k=2$**: Relaxing through vertex 2 evaluates $d[0][2] + d[2][3] = 8 + 1 = 9$. Since $9 < 10$, $d[0][3]$ improves from 10 to 9.
  - This demonstrates how dynamic programming constructs optimal paths from previously computed optimal sub-paths.
- **`alt_dijkstra_all_pairs.c`**: Computes the exact same all-pairs distance table by executing Single-Source Dijkstra from each of the $V$ vertices using a CSR graph and binary min-heap.
  - Verifies exact match against Floyd-Warshall on the 4-vertex graph.

## 2. Empirical Benchmark Measurements (Median of 3 Runs)
Graphs generated with average degree $\sim 3$ edges/vertex (fixed seed `srand(42)`):

| Vertices ($V$) | Edges ($E$) | Floyd-Warshall ($O(V^3)$) | All-Pairs Dijkstra ($O(V(V+E)\log V)$) | Speedup Ratio |
| :--- | :--- | :--- | :--- | :--- |
| **30** | 90 | 0.000032 s | 0.000040 s | 0.81x (FW lower constant factor) |
| **200** | 600 | 0.005545 s | 0.002659 s | 2.09x (Dijkstra faster) |
| **400** | 1200 | 0.042110 s | 0.011209 s | 3.76x (Dijkstra faster) |
| **800** | 2400 | 0.396972 s | 0.056525 s | **7.02x (Dijkstra faster)** |

### Algorithmic Insight:
- On small dense matrices ($V \le 30$), Floyd-Warshall's cache-friendly contiguous array strides and branch-free inner loop achieve competitive raw execution times.
- As $V$ grows to 800 on realistic sparse road networks ($E \approx 3V$), Floyd-Warshall grows cubically ($800^3 \approx 5.12 \times 10^8$ ops), whereas repeated Dijkstra on CSR lists scales near-quadratically ($V^2 \log V$), executing **7 times faster**.
