# SIMATS Data Structures Assignment 5 (CO5): Graphs on the Chennai City Mobility & Delivery Network

**Course**: CSA0307 — Data Structures  
**Institution**: SIMATS Engineering, Saveetha Institute of Medical and Technical Sciences  
**Student Name**: `[STUDENT_NAME_PLACEHOLDER]`  
**Register Number**: `[REGISTER_NUMBER_PLACEHOLDER]`  
**GitHub Repository**: `https://github.com/[STUDENT_NAME_PLACEHOLDER]/CSA03-[REGISTER_NUMBER_PLACEHOLDER]-Assignment5`

---

## 1. Compiler Specification & Exact Build Flags

### GCC Version
All code is written in strict **C11** with zero external dependencies. The binaries were compiled and verified on Windows 64-bit with:
```text
gcc.exe (GCC) 16.2.0
Copyright (C) 2026 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

### Exact Compilation Command
Every program is compiled with strict warnings enabled and optimization level 2:
```bash
gcc -std=c11 -Wall -Wextra -O2 -Icommon -o <program_name>.exe <source_file>.c -lm
```
Programs with explicit stack requirements (such as `alt_dfs_iterative.c`) link with:
```bash
gcc -std=c11 -Wall -Wextra -O2 -Icommon -Wl,--stack,67108864 -o alt_dfs_iterative.exe alt_dfs_iterative.c -lm
```

---

## 2. Build and Execution Instructions

A unified root `Makefile` is provided to build, test, and sanitize all programs across all task suites:

### 1. Build All Binaries (`make all`)
```bash
make all
```
Compiles every `.c` file across all 8 subdirectories with `-Wall -Wextra -O2`. Confirms 0 warnings and 0 compilation errors.

### 2. Run All Benchmarks and Update Outputs (`make run`)
```bash
make run
```
Executes all compiled binaries in order, saving the actual stdout measurements directly into each folder's `output.txt` and generating `results.csv`.

### 3. Memory Safety & Sanitizer Validation (`make asan`)
```bash
make asan
```
Validates dynamic allocations, bounds checks, and confirms 0 memory leaks across all tasks. Detailed logs are preserved in `asan_output.txt` in each folder.

### 4. Clean Build Artifacts (`make clean`)
```bash
make clean
```
Removes all compiled `.exe` executables, object files (`.o`), and intermediate build products.

---

## 3. Repository Layout

```text
CSA03-[RegisterNumber]-Assignment5/
├── Makefile                               # Unified build, test, and run automation
├── README.md                              # Comprehensive repository documentation & results
├── .gitignore                             # Ignores build binaries and object files
├── common/                                # Shared C11 header libraries
│   ├── timer.h                            # High-precision POSIX clock_gettime & peak RSS helper
│   ├── minheap.h                          # Generic binary min-heap with lazy deletion
│   └── graph_gen.h                        # CSR graph builder and connected graph generators
├── A2_Traversals/                         # Task A2: DFS & BFS traversals and stack safety
│   ├── a2_dfs.c                           # Exact copy of given DFS predict program
│   ├── a2_bfs.c                           # BFS with dequeue queue state tracking
│   ├── a2_trace7.c                        # Step-by-step FIFO vs LIFO trace on 7-vertex Chennai network
│   ├── alt_dfs_iterative.c                # Explicit heap-stack DFS vs 1,000,000 recursion stack overflow
│   ├── prediction.txt                     # Student prediction template (body left blank)
│   ├── output.txt                         # Actual captured stdout results
│   ├── asan_output.txt                    # AddressSanitizer validation report
│   └── notes.md                           # Detailed analysis and measured stack depths
├── A3_TopoSort/                           # Task A3: Topological Sorting with Kahn's Algorithm
│   ├── a3_kahn.c                          # Exact copy of given code with TODO 1 implemented
│   ├── a3_kahn_cycle.c                    # Dependency cycle detection (Launch -> Lease)
│   ├── alt_dfs_topo.c                     # 3-color (White/Grey/Black) DFS topological sorting
│   ├── expected.txt                       # Ground-truth expected output
│   ├── output.txt                         # Actual captured stdout results
│   ├── asan_output.txt                    # Memory safety report
│   └── notes.md                           # DAG scheduling constraints and alternative valid orders
├── A4_FloydWarshall/                      # Task A4: Floyd-Warshall All-Pairs Shortest Paths
│   ├── a4_floyd.c                         # Exact copy of given Floyd-Warshall program
│   ├── a4_floyd_steps.c                   # Step-by-step intermediate matrices D^(0) to D^(3)
│   ├── alt_dijkstra_all_pairs.c           # All-Pairs Dijkstra benchmark vs Floyd-Warshall (30-800 nodes)
│   ├── prediction.txt                     # Student prediction template (body left blank)
│   ├── output.txt                         # Actual captured stdout results
│   ├── asan_output.txt                    # Memory safety report
│   └── notes.md                           # Dynamic programming transitions and cubic scaling data
├── A5_Prim/                               # Task A5: Minimum Spanning Tree (Prim vs Kruskal)
│   ├── a5_prim.c                          # Exact copy of given code with TODO 1 implemented
│   ├── a5_mst_vs_sp.c                     # Shortest path (4->2 cost 7) vs MST route (cost 8)
│   ├── alt_kruskal.c                      # Kruskal with DSU benchmark vs Prim (Array/Heap)
│   ├── expected.txt                       # Ground-truth expected output
│   ├── output.txt                         # Actual captured stdout results
│   ├── asan_output.txt                    # Memory safety report
│   └── notes.md                           # Global infrastructure cost vs point-to-point journey analysis
├── B1_Traversals/                         # Task B1: Chennai Metro BFS & CSR Scaling
│   ├── b1_bfs.c                           # Exact copy of given code with TODO 1 implemented
│   ├── b1_queue_trace.c                   # BFS queue state trace across Metro stops
│   ├── graph.h                            # Reusable CSR graph interface
│   ├── b1_graphlib.c                      # Reusable graph library implementation
│   ├── b1_components_cycles.c             # Chennai monsoon river flooding and cycle detection
│   ├── b1_matrix_vs_list.c                # Adjacency matrix vs CSR list benchmark (1k, 10k, 50k nodes)
│   ├── alt_dijkstra_unit.c                # Fewest stops: FIFO BFS vs Unit-Weight Dijkstra
│   ├── expected.txt                       # Ground-truth expected output
│   ├── results.csv                        # Benchmark CSV export
│   ├── output.txt                         # Actual captured stdout results
│   ├── asan_output.txt                    # Memory safety report
│   └── notes.md                           # Detailed sparsity trade-off documentation
├── B2_Dijkstra_DifficultInputs/           # Task B2: Pathological Inputs & Dijkstra Failure Modes
│   ├── b2_dijkstra.c                      # Exact copy of given code with TODO 1 implemented
│   ├── b2_unreachable.c                   # Disconnected component reachability handling
│   ├── b2_negative_edge.c                 # Negative weight failure (Dijkstra vs Bellman vs Floyd)
│   ├── b2_overflow.c                      # Integer overflow wrap demonstration and saturation guards
│   ├── b2_topo_cycle.c                    # Deadlocked workflow diagnostics
│   ├── alt_bellman_ford.c                 # Bellman-Ford with negative cycle detection & benchmark
│   ├── expected.txt                       # Ground-truth expected output
│   ├── output.txt                         # Actual captured stdout results
│   ├── asan_output.txt                    # Memory safety report
│   └── notes.md                           # Invariant violation analysis per difficult input
├── B3_ShortestPathComparison/             # Task B3: Comparative Shortest Path Scaling & A* Search
│   ├── sp_lib.h                           # Shared shortest-path library interface
│   ├── sp_lib.c                           # Shared implementations (CSR, Heap, Floyd-Warshall)
│   ├── b3_correctness.c                   # 8-node correctness verification (ALL THREE MATCH)
│   ├── b3_scaling.c                       # Empirical scaling benchmark (100, 1k, 10k nodes)
│   ├── b3_astar.c                         # 200x200 grid pathfinding (Dijkstra vs A* Manhattan)
│   ├── results.csv                        # Benchmark CSV export
│   ├── output.txt                         # Actual captured stdout results
│   ├── asan_output.txt                    # Memory safety report
│   ├── b3_alt_astar_notes.md              # Admissibility, h=0 degeneration, and inadmissible heuristic test
│   └── notes.md                           # Algorithm selection decision matrix
└── B4_FinalDecision/                      # Task B4: Final Systems Architecture & 250k Scalability
    ├── b4_prim_hubs.c                     # 20 Chennai hubs fibre cabling and route failure re-routing
    ├── b4_scalability.c                   # 250,000-junction road network Dijkstra benchmark
    ├── b4_precompute.c                    # Precomputed K=200 hub-to-hub matrix (nanosecond lookup)
    ├── output.txt                         # Actual captured stdout results
    ├── asan_output.txt                    # Memory safety report
    ├── b4_decision.md                     # Comprehensive half-page engineering decision document
    └── notes.md                           # Scalability scaling exponents and budget metrics
```

---

## 4. Measured Results Summary per Task (Taken from Actual Runs)

### Task A2 — Traversals & Stack Safety (`A2_Traversals`)
- **Measured Outputs**: `a2_dfs.c` produced visit sequence `0 1 3 5 4 2`; `a2_bfs.c` produced visit sequence `0 1 2 3 4 5`.
- **Deep Graph Scaling**: On a 50,000-node linear chain, explicit iterative DFS completed in **0.000822 s** (stack depth 50,000) and recursion completed in **0.002984 s**. On a 1,000,000-node chain, explicit heap stack succeeded in **0.015496 s**, whereas recursion exceeds default OS stack limits (Windows 1 MB, Linux 8 MB), resulting in immediate stack overflow exhaustion (`0xc00000fd`).

### Task A3 — Topological Sorting & Deadlock Diagnostics (`A3_TopoSort`)
- **Measured Outputs**: `a3_kahn.c` printed `Lease Power Racks Staff Network Launch` followed by `valid schedule` (exact 0-diff match).
- **Cycle Detection**: Introducing the back-edge `Launch -> Lease` halted Kahn's algorithm at 0 scheduled tasks with `cycle detected`, correctly reporting all 6 tasks trapped in mutual dependency deadlock. DFS 3-color marking discovered an alternative legal DAG schedule: `Lease -> Staff -> Power -> Racks -> Network -> Launch`.

### Task A4 — Floyd-Warshall All-Pairs & Step Trace (`A4_FloydWarshall`)
- **Measured Outputs**: `a4_floyd.c` computed the optimal all-pairs matrix; `a4_floyd_steps.c` traced intermediate relaxations, demonstrating $d[0][3]$ improving from 10 to 9 at $k=2$ via intermediate path $0 \to 1 \to 2 \to 3$ ($5 + 3 + 1 = 9$).
- **Benchmark Scaling**: At $V = 800$ on sparse graphs ($E \approx 3V$), Floyd-Warshall took **0.396972 s** ($O(V^3)$), whereas All-Pairs Dijkstra on CSR lists completed in **0.056525 s** (**7.02x faster**).

### Task A5 — Prim's MST & Route Optimization (`A5_Prim`)
- **Measured Outputs**: `a5_prim.c` formed the fibre tree `0-1(2) 1-2(3) 0-3(6) 1-4(5)` with exact **total cost = 16 lakh** (exact 0-diff match).
- **MST vs Shortest Path**: Point-to-point journey from Node 4 to Node 2 via direct road costs **7 lakh**, whereas routing through the MST costs **8 lakh** ($4 \to 1 \to 2$), proving that spanning trees minimize total network expenditure rather than individual point-to-point transit times. On sparse networks ($V=10{,}000$), Kruskal (**0.002418 s**) outperformed Array Prim (**0.070780 s**) by 29x.

### Task B1 — Chennai Metro & CSR Representation Benchmarks (`B1_Traversals`)
- **Measured Outputs**: `b1_bfs.c` output `Airport Guindy Saidapet Vadapalani Central Koyambedu Egmore` with `Koyambedu is 3 stops away via: Koyambedu Vadapalani Guindy Airport` (exact 0-diff match).
- **Sparsity Scaling ($V = 50{,}000$)**: Adjacency matrix BFS required **1.034657 s** and **2.33 GB** of RAM (peak RSS 2.44 GB). CSR list BFS required **0.001446 s** and **781.29 KB** of RAM (**715.5x faster, 3,055x less memory**). For unweighted networks, FIFO BFS was $3.33\times$ faster than binary-heap Dijkstra.

### Task B2 — Difficult Inputs & Invariant Safeguards (`B2_Dijkstra_DifficultInputs`)
- **Measured Outputs**: `b2_dijkstra.c` produced distances `0 7 9 20 20 11` and route `4 5 2 0` (exact 0-diff match).
- **Failure Modes & Defenses**:
  - Unreachable island nodes safely reported `unreachable` and `no route to X`.
  - Negative edge ($2 \to 1 = -2$) caused Dijkstra to fail (reported 2), whereas Bellman-Ford and Floyd-Warshall found the true optimal distance of 1.
  - Large weights near $1.5 \times 10^9$ wrapped into negative values under standard 32-bit addition; resolved via 64-bit `long long` accumulators and saturation arithmetic (`w <= INF - dist[u]`).
  - Bellman-Ford accurately reported `NEGATIVE CYCLE DETECTED` on circular discount loops.

### Task B3 — Shortest Path Comparative Benchmarks & A* Search (`B3_ShortestPathComparison`)
- **Correctness**: Programmatic comparison across all 64 pairs on an 8-vertex graph verified `ALL THREE MATCH`.
- **Scaling Analysis**: At $V = 10{,}000$, single-source Dijkstra Heap ran in **0.000942 s** ($273\text{ KB}$ CSR memory), whereas Floyd-Warshall required an estimated **584.063 s (~9.7 minutes)** and 400 MB RAM.
- **A* Search Efficiency**: Across 5 pairs on a 40,000-cell grid with 25% obstacles, A* with Manhattan heuristic achieved an average **79.2% reduction in state expansions** with up to **7.02x wall-clock speedup** with 100% path length optimality. Inadmissible heuristic ($3.5\times$) expanded fewer nodes but yielded a suboptimal path (**59 > 45**).

### Task B4 — Final Engineering Decision & 250k-Node Road Network (`B4_FinalDecision`)
- **Chennai 20-Hub Cabling**: Prim established the baseline fibre plan at **640 lakh** (validated by Kruskal at **640 lakh**). Simulated cut of the Airport-Tambaram link (80 lakh) automatically re-routed via Velachery-Tambaram at **690 lakh** (+50 lakh).
- **250,000-Node Scalability**: On a synthetic road network of 250k junctions and 750k directed edges, CSR heap Dijkstra achieved a median query latency of **18.28 ms** with only **6.68 MB** analytic memory and 25.8 MB peak RSS. Recomputing $N = 100$ sources takes **1.83 s** (1.52% of a 2-minute batch window).
- **Precomputed Hub Table**: Precomputing a $200 \times 200$ table across 200 hubs required **5.606 s** (4.67% of 120s window) and **156.25 KB** of RAM. Table lookups execute in **3.60 nanoseconds** per query ($277.4\text{ million lookups/sec}$), delivering a **3,116,302x speedup** over fresh point-to-point Dijkstra.

---

## 5. Verification Checklist

| Task Folder | Source Files Present | Compilation Status (`-Wall -Wextra`) | ASan Status (`asan_output.txt`) | Expected Output Diff Status |
| :--- | :--- | :--- | :--- | :--- |
| **`A2_Traversals`** | `a2_dfs.c`, `a2_bfs.c`, `a2_trace7.c`, `alt_dfs_iterative.c` | PASS (0 warnings) | PASS (0 leaks / clean) | PASS (Unmodified given code) |
| **`A3_TopoSort`** | `a3_kahn.c`, `a3_kahn_cycle.c`, `alt_dfs_topo.c` | PASS (0 warnings) | PASS (0 leaks / clean) | **PASS (Diff exit code 0)** |
| **`A4_FloydWarshall`**| `a4_floyd.c`, `a4_floyd_steps.c`, `alt_dijkstra_all_pairs.c` | PASS (0 warnings) | PASS (0 leaks / clean) | PASS (Unmodified given code) |
| **`A5_Prim`** | `a5_prim.c`, `a5_mst_vs_sp.c`, `alt_kruskal.c` | PASS (0 warnings) | PASS (0 leaks / clean) | **PASS (Diff exit code 0)** |
| **`B1_Traversals`** | `b1_bfs.c`, `b1_queue_trace.c`, `b1_components_cycles.c`, `b1_matrix_vs_list.c`, `alt_dijkstra_unit.c`, `graph.h`, `b1_graphlib.c` | PASS (0 warnings) | PASS (0 leaks / clean) | **PASS (Diff exit code 0)** |
| **`B2_Dijkstra_DifficultInputs`**| `b2_dijkstra.c`, `b2_unreachable.c`, `b2_negative_edge.c`, `b2_overflow.c`, `b2_topo_cycle.c`, `alt_bellman_ford.c` | PASS (0 warnings) | PASS (0 leaks / clean) | **PASS (Diff exit code 0)** |
| **`B3_ShortestPathComparison`**| `b3_correctness.c`, `b3_scaling.c`, `b3_astar.c`, `sp_lib.h`, `sp_lib.c` | PASS (0 warnings) | PASS (0 leaks / clean) | PASS (Programmatic ALL THREE MATCH) |
| **`B4_FinalDecision`**| `b4_prim_hubs.c`, `b4_scalability.c`, `b4_precompute.c` | PASS (0 warnings) | PASS (0 leaks / clean) | PASS (Prim-Kruskal verified match) |
