# Task B4 Notes — Final Engineering Decisions & 250k-Node Benchmarks

## 1. What the Programs Show
- **`b4_prim_hubs.c`**:
  - Models dedicated communication cable laying between 20 Chennai urban delivery hubs (`Hub01_Central` through `Hub20_Koyambedu`).
  - Allowed road/terrain links: 165 out of 190 total possible pairs (~15% disallowed due to environmental/zoning barriers).
  - Baseline Prim MST total cost: **640 lakh** (independently validated by Kruskal at **640 lakh**).
  - Fault Simulation 1 (severing most expensive link `Airport <-> Tambaram`, 80 lakh): Re-routed MST cost is **690 lakh**, dynamically adding `Velachery <-> Tambaram` (130 lakh) to restore spanning tree connectivity.
  - Fault Simulation 2 (severing mid-weight link `Adyar <-> BesantNagar`, 20 lakh): Re-routed MST cost remains **640 lakh**, seamlessly substituting alternative corridor `Thiruvanmiyur <-> BesantNagar` (20 lakh).
- **`b4_scalability.c`**:
  - Tests high-performance binary-heap Dijkstra across 10k, 50k, 100k, and 250k junction road networks (~750,000 directed edges).
  - Demonstrates that CSR list representation requires only **6.68 MB** of memory at 250,000 vertices with a median single-source query latency of **18.28 ms**.
  - Evaluates batch recomputation feasibility within a 2-minute (120 s) traffic update cycle:
    - $N = 100$ sources require **1.83 s** (1.52% of budget).
    - $N = 1,000$ sources require **18.28 s** (15.23% of budget).
- **`b4_precompute.c`**:
  - Precomputes a $200 \times 200$ distance table for $K = 200$ major distribution hubs across the 250k-node network.
  - Total recomputation time is **5.606 seconds** (4.67% of the 120-second window).
  - Memory footprint for distance matrix is **156.25 KB**.
  - Table lookup executes in **3.60 nanoseconds** per query ($277.4\text{ million queries/sec}$), delivering a **3,116,302x speedup** over fresh point-to-point Dijkstra (median 11.23 ms).

## 2. Key Engineering Metrics Table
Extracted directly from `B4_FinalDecision/output.txt`:

| Scale ($V$) | Edges ($E$) | Analytic CSR Mem | Heap Mem | Median Query Latency | Peak Process RSS |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **10,000** | 30,000 | 0.27 MB | 0.31 MB | 0.9000 ms | 4,840 KB |
| **50,000** | 150,000 | 1.34 MB | 1.53 MB | 3.2542 ms | 8,248 KB |
| **100,000** | 300,000 | 2.67 MB | 3.05 MB | 10.5862 ms | 12,424 KB |
| **250,000** | 750,000 | 6.68 MB | 7.63 MB | 18.2768 ms | 25,812 KB |
