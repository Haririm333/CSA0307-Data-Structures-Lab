# Task A3 Notes — Topological Sorting & Cycle Detection

## 1. What the Programs Show
- **`a3_kahn.c`**: Implements Kahn's in-degree reduction algorithm (BFS-based) on Chennai delivery-hub preparation tasks.
  - The tasks are: `0: Lease`, `1: Power`, `2: Racks`, `3: Network`, `4: Staff`, `5: Launch`.
  - Initial in-degrees: `Lease` (0), `Power` (1), `Racks` (1), `Network` (2), `Staff` (1), `Launch` (2).
  - Tasks with in-degree 0 are queued and scheduled. Every finished task decrements its downstream dependents' in-degrees.
  - Result: `Lease Power Racks Staff Network Launch` (`valid schedule`).
- **`a3_kahn_cycle.c`**: Introduces a dependency cycle by adding the back-edge `Launch -> Lease`.
  - In a cycle, every vertex in the cycle has an in-degree of at least 1.
  - Kahn's algorithm terminates early with `done = 0 < V`, correctly identifying a deadlocked workflow.
  - Identifies unresolved tasks: `Lease`, `Power`, `Racks`, `Network`, `Staff`, `Launch`.
- **`alt_dfs_topo.c`**: Implements 3-colour DFS topological sorting (`WHITE` = unvisited, `GREY` = on active recursion path, `BLACK` = finished).
  - When an edge encounters a `GREY` node, a back-edge is proven, immediately halting and signaling a cycle.

## 2. Multiple Valid Topological Orders
A directed acyclic graph (DAG) does **not** have a single unique topological order unless every pair of vertices has a direct or indirect path between them (a Hamiltonian path).

For the Chennai hub setup graph, multiple valid orderings exist:
1. **Order 1 (Observed via Kahn's BFS queue)**:
   `Lease -> Power -> Racks -> Staff -> Network -> Launch`
2. **Order 2 (Observed via 3-Color DFS Post-Order)**:
   `Lease -> Staff -> Power -> Racks -> Network -> Launch`
3. **Order 3 (Alternative legal schedule)**:
   `Lease -> Racks -> Power -> Staff -> Network -> Launch`

### Verification of Validity:
In all orders:
- `Lease` finishes before `Power`, `Racks`, and `Staff`.
- Both `Power` and `Racks` finish before `Network`.
- Both `Network` and `Staff` finish before `Launch`.
All precedence constraints are strictly honoured.
