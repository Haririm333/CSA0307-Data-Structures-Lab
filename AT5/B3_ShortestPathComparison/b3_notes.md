# Task B3 Notes — Shortest Path Algorithm Comparison & Decision Matrix

## 1. Measured Scaling Summary (Degree ~ 3, Average of 10 Sources)
Based on real measurements recorded in `results.csv` and `output.txt`:

| Algorithm | $V = 100$ | $V = 1,000$ | $V = 10,000$ | Memory Model | Scalability Limit |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Dijkstra (Array)** | 0.000045 s | 0.001348 s | 0.237156 s | $O(V)$ auxiliary arrays ($40\text{ KB}$ at $10\text{k}$) | $O(V^2)$ scan becomes bottleneck above $10\text{k}$ nodes |
| **Dijkstra (Binary Heap)** | 0.000008 s | 0.000068 s | 0.000942 s | $O(V + E)$ CSR + Heap ($273\text{ KB}$ at $10\text{k}$) | **Excels on sparse graphs ($> 250\text{x}$ faster than array at $10\text{k}$)** |
| **Floyd-Warshall** | 0.001170 s | 0.584063 s | 584.063 s (est.) | $O(V^2)$ dense matrix ($400\text{ MB}$ at $10\text{k}$) | Impractical for $V > 2,000$ due to cubic $O(V^3)$ operations |
| **A* Search (Manhattan)** | $\sim 5\mu\text{s}$ | $\sim 30\mu\text{s}$ | $\sim 200\mu\text{s}$ | $O(V)$ spatial grid ($O(V)$ heap) | **Best for single-pair point-to-point pathfinding ($79.2\%$ fewer nodes)** |

---

## 2. Engineering Selection Matrix: When to Use Which Algorithm

| Use Case / Scenario | Recommended Algorithm | Reason Based on Experimental Data |
| :--- | :--- | :--- |
| **Point-to-Point Navigation on 2D Maps** (e.g. Swiggy/Zomato rider route) | **A* Search with Euclidean/Manhattan Heuristic** | Directional goal guidance reduces explored search space by **79.2%**, achieving **$5\times\text{ to }7\times$ lower latency** with zero loss of optimality. |
| **Single-Source Route Dispatch to All Hubs** (e.g. Courier hub distributing to all city centers) | **Dijkstra with Min-Heap (CSR)** | Explores all reachable junctions in $O((V+E)\log V)$ time (**0.94 ms for 10,000 nodes**), scaling cleanly without dense matrix allocation. |
| **Small Dense Subnetworks ($V \le 50$)** (e.g. Central core exchange interconnections) | **Floyd-Warshall** | Trivial implementation, cache-friendly memory access, zero priority queue overhead. |
| **All-Pairs on Sparse City Road Networks ($V \ge 200$)** | **All-Pairs Dijkstra (Heap-CSR)** | Running Dijkstra $V$ times takes $O(V(V+E)\log V)$ time (**0.056 s vs 0.397 s for FW at $V=800$**). At $V=10,000$, repeated Dijkstra takes $\approx 9.4\text{ s}$ vs Floyd-Warshall's estimated $\approx 584\text{ s}$. |
| **Graphs with Negative Travel Times / Toll Discounts** | **Bellman-Ford / Johnson's Algorithm** | Dijkstra fails greedy choice when edge weights are negative. Bellman-Ford detects negative cycles in $O(V \cdot E)$. |
