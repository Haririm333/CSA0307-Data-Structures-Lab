# Engineering Decision Document: Chennai City Mobility & Delivery Network Routing Architecture

**Reference Outputs**: `B4_FinalDecision/output.txt`, `B3_ShortestPathComparison/output.txt`, `B1_Traversals/output.txt`, `A5_Prim/output.txt`.

---

### 1. Graph Representation: Compressed Sparse Row (CSR) Adjacency List
For the Chennai urban road network of 250,000 junctions and 750,000 directed edges, dense adjacency matrix representation is completely unviable, requiring $250{,}000^2 \times 1\text{ byte} = \mathbf{62.5\text{ GB}}$ of memory (even at 50,000 nodes, calloc failed or took 2.33 GB as measured in `B1_Traversals/output.txt`). 
In contrast, our **CSR Adjacency List** stores the entire 250,000-node network in **6.68 MB** of analytic memory (offset array: 1.00 MB, adjacency: 3.00 MB, edge weights: 3.00 MB) with an operating peak RSS of **25.81 MB**. Furthermore, traversals on CSR run in strictly linear $O(V + E)$ time (**1.45 ms** on CSR vs **1,034.66 ms** on matrix at 50,000 nodes, delivering a measured **715.5x speedup**). CSR is selected as the primary graph representation.

### 2. Point-to-Point Rider Navigation: A* Search with Geometric Heuristic
For ad-hoc point-to-point delivery routing between arbitrary addresses, uniform-cost Dijkstra explores radial waves across unpromising quadrants. Our benchmarks on the 40,000-cell mobility map (`B3_ShortestPathComparison/output.txt`) prove that **A* Search with an admissible Manhattan/Euclidean distance heuristic** reduces the number of explored vertices by an average of **79.2%** (e.g. Pair 4 explored 3,458 cells vs Dijkstra's 23,744 cells) while guaranteeing **identical optimal path length** (208 steps) and achieving a **2.13x to 7.02x wall-clock query speedup**. A* with a consistent coordinate-distance heuristic is selected for all on-demand delivery dispatch engines.

### 3. Hub-to-Hub Distance Matrix: All-Pairs Dijkstra vs Floyd-Warshall
For calculating all-pairs travel times between the city's 30 distribution hubs:
- **Floyd-Warshall** exhibits $O(V^3)$ complexity. At $V = 1{,}000$, FW required **0.584 s**, scaling at $V = 10{,}000$ to an estimated **584.06 s (~9.7 minutes)** and requiring 400 MB of flat array storage (`B3_ShortestPathComparison/output.txt`).
- **All-Pairs Dijkstra (CSR with Binary Min-Heap)** scales as $O(V(V+E)\log V)$. Computing single-source trees from all 30 hubs on the 250,000-junction graph takes $30 \times 18.28\text{ ms} = \mathbf{0.548\text{ seconds}}$.
Repeated heap Dijkstra on CSR lists is chosen for all hub-to-hub distance calculations, outperforming Floyd-Warshall by multiple orders of magnitude.

### 4. Dedicated Fibre Infrastructure Cabling: Prim vs Kruskal
For laying dedicated underground communication fibre interconnecting the 20 Chennai hubs (`B4_FinalDecision/output.txt`):
- **Prim's Algorithm** established the baseline minimum spanning tree at an infrastructure cost of **640 lakh** (verified identically by Kruskal at **640 lakh**).
- Under simulated route failure (severing the most expensive link `Airport <-> Tambaram`, 80 lakh), dynamic re-routing via Prim automatically restored full tree connectivity via `Velachery <-> Tambaram` (130 lakh) at a total re-routed cost of **690 lakh** (+50 lakh increase).
- Performance trade-offs (`A5_Prim/output.txt`): On sparse networks ($V=10{,}000, E=30{,}000$), Kruskal (**2.42 ms**) and Heap-Prim (**2.79 ms**) outperform Array-Prim (**70.78 ms**). On dense graphs ($V=2{,}000, E=1{,}000{,}000$), Array-Prim dominates at **4.01 ms** vs Kruskal's **73.16 ms**. For planning infrastructure networks where candidate corridors are sparse, Kruskal/Heap-Prim is chosen.

### 5. Precomputation Architecture & Batch Update Feasibility
For real-time dispatch systems needing instant transit estimates among $K = 200$ major delivery hubs on the 250,000-node city network (`B4_FinalDecision/output.txt`):
- **Precomputation Cost**: Precomputing the full $200 \times 200$ distance table via 200 Dijkstra passes takes **5.606 seconds** (**28.03 ms per hub**). This consumes merely **4.67% of the 2-minute (120 s) traffic refresh window**, leaving 114.4 seconds free for CPU scheduling.
- **Storage**: Distance-only table requires **156.25 KB**; full next-hop routing requires **312.50 KB** (easily pinned in L3 CPU cache).
- **Online Query Latency**: Table lookup executes in **3.60 nanoseconds** ($277.4\text{ million lookups/sec}$), outperforming fresh point-to-point Dijkstra (**11.23 ms**) by a factor of **3,116,302x**.
A precomputed hub-to-hub distance cache refreshed asynchronously every 2 minutes is selected for production deployment.
