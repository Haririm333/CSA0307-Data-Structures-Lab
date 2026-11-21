# Task A2 Notes — Graph Traversals & Stack Safety

## 1. What the Programs Show
- **`a2_dfs.c`**: Demonstrates recursive Depth-First Search on a 6-vertex undirected graph. Starting at vertex 0, DFS explores down each path as deeply as possible before backtracking. The observed visit sequence is `0 1 3 5 4 2`.
- **`a2_bfs.c`**: Demonstrates Breadth-First Search on the exact same graph. Using a FIFO queue, BFS discovers nodes level-by-level (by shortest number of hops). The observed dequeuing and visit sequence is `0 1 2 3 4 5`.
- **`a2_trace7.c`**: Traces step-by-step queue transitions (FIFO in BFS) and stack transitions (LIFO in DFS) on a 7-vertex Chennai delivery network (`Central`, `Egmore`, `T.Nagar`, `Guindy`, `Adyar`, `Velachery`, `Tambaram`).
- **`alt_dfs_iterative.c`**: Compares recursion on the machine call stack versus an explicit heap-allocated stack for very deep graphs.

## 2. Measured Results & Deep Graph Scaling
- **50,000-Junction Linear Chain**:
  - Explicit Iterative DFS: **0.000822 seconds**, max stack depth = 50,000.
  - Recursive Call-Stack DFS: **0.002984 seconds**, max recursion depth = 50,000.
- **1,000,000-Junction Linear Chain**:
  - Explicit Iterative DFS: **0.015496 seconds**, max stack depth = 1,000,000. Completed successfully without memory or stack faults.
  - Recursive DFS: Guarded with `--crash`. On standard OS environments, 1,000,000 recursive frames require approximately $1{,}000{,}000 \times 48\text{ bytes} \approx 48\text{ MB}$.
    - Standard Linux thread stack limit (`ulimit -s`): **8192 KB (8 MB)**.
    - Standard Windows thread stack limit: **1024 KB (1 MB)**.
    - Uncontrolled recursion to depth 1,000,000 causes immediate stack overflow exhaustion (`SIGSEGV` / `EXCEPTION_STACK_OVERFLOW`).
    - The explicit heap-allocated stack dynamically stores state in user space, rendering it immune to OS call-stack limits.
