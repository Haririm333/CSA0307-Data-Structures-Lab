# Task B2 Notes — Edge Cases, Invariant Violations & Algorithmic Limits

## 1. Reachability Assumption (`b2_unreachable.c`)
- **Assumption Exposed**: Dijkstra assumes that the graph is connected, or that any unreached vertex will eventually be reached. When a disconnected component exists (such as offshore island hubs 4 and 5), unreached vertices retain `dist = INT_MAX`.
- **Observed Result**: The priority queue empties early (at round 4), leaving vertices 4 and 5 untouched. A robust implementation must identify `dist[v] == INT_MAX` and report `"unreachable"` and `"no route to X"` rather than printing numeric overflow values or garbage predecessor pointers.

## 2. Non-Negative Weights Assumption (`b2_negative_edge.c`)
- **Assumption Exposed**: Dijkstra's greedy choice property relies on the invariant that path costs monotonically increase as edges are traversed ($w(u, v) \ge 0$). Once a vertex is marked "done" (extracted from the priority queue), Dijkstra assumes its distance is optimal and never re-relaxes its incident edges.
- **Observed Result**: On the 3-vertex graph with edges $0 \to 1$ (+2), $0 \to 2$ (+3), and $2 \to 1$ (-2), Dijkstra extracts vertex 1 first with cost 2 and marks it done. It subsequently explores vertex 2 with cost 3, discovering a cheaper route to 1 ($3 + (-2) = 1$), but is prohibited by its greedy invariant from updating vertex 1. Consequently, Dijkstra reports `dist[1] = 2` (FAILURE). Both Bellman-Ford and Floyd-Warshall correctly report `dist[1] = 1`.

## 3. Finite Integer Range & Overflow Assumption (`b2_overflow.c`)
- **Assumption Exposed**: Standard integer addition assumes mathematical $\mathbb{Z}$, but machine arithmetic uses fixed 32-bit two's complement ($\le 2^{31}-1$). In road networks with large edge weights (e.g. $1{,}500{,}000{,}000$ minutes), `dist[u] + w` overflows `INT_MAX`.
- **Observed Result**: Unchecked addition wraps into a negative integer ($-1{,}294{,}967{,}296$), leading Dijkstra to falsely believe an exorbitant path is shorter than the source. Furthermore, adding any weight to unreached vertices (`INF + w`) overflows. Fixes verified: (a) 64-bit `long long` accumulators prevent overflow entirely ($3{,}000{,}000{,}000$); (b) saturation checks (`w <= INF - dist[u]`) clamp distance at `INT_MAX`; (c) skipping unvisited nodes (`dist[u] != INF`) eliminates invalid relaxations.

## 4. Acyclicity Assumption in Topological Sorting (`b2_topo_cycle.c`)
- **Assumption Exposed**: Topological sorting (Kahn's algorithm) strictly requires a Directed Acyclic Graph (DAG), assuming at least one vertex has in-degree 0 at every stage.
- **Observed Result**: When the back-edge `Launch -> Lease` is introduced, a mutual deadlock loop is created. All 6 tasks remain with in-degrees $\ge 1$. The queue initializes empty, 0 tasks are scheduled (`0 / 6`), and Kahn's algorithm terminates immediately with `"CYCLE DETECTED (DEADLOCK)"`, successfully identifying all stuck tasks awaiting impossible prerequisites.

## 5. Negative Cycle Detection & General Weights (`alt_bellman_ford.c`)
- **Assumption Exposed**: While Dijkstra fails on negative edges, Bellman-Ford can handle arbitrary negative edge weights by relaxing all edges $V-1$ times. However, if a negative weight cycle exists, distances can decrease indefinitely to $-\infty$.
- **Observed Result**: On the negative-edge graph without cycles, Bellman-Ford correctly outputs `dist[1] = 1`. On a graph containing a negative loop ($0 \to 1 \to 2 \to 0$ summing to -2), Bellman-Ford detects relaxation on the $V$-th pass and reports `"NEGATIVE CYCLE DETECTED"`. On the Chennai 6-node network, Bellman-Ford matches Dijkstra (`0 7 9 20 20 11`).
