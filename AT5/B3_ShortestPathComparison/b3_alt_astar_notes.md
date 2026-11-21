# Task B3 Notes — A* Heuristic Search, Admissibility & Edge Cases

## 1. Algorithmic Invariants of A* Search
A* search guides exploration using an evaluation function:
$$f(n) = g(n) + h(n)$$
where:
- $g(n)$ is the exact cost from the start node to node $n$.
- $h(n)$ is a heuristic estimate of the cost from node $n$ to the goal.

### Admissibility Condition:
For A* tree and graph search to guarantee finding the strictly optimal shortest path, the heuristic must be **admissible**:
$$h(n) \le h^*(n) \quad \forall n$$
where $h^*(n)$ is the true shortest-path distance from $n$ to the goal. On a 4-connected grid with unit step weights, the **Manhattan distance** $h(x, y) = |x - x_g| + |y - y_g|$ is:
1. **Admissible**: Every 4-directional move changes $x$ or $y$ by at most 1 unit; no path can reach the goal faster than the coordinate delta. Obstacles only increase path length, never shorten it.
2. **Consistent (Monotonic)**: For adjacent cells $n, n'$, $|h(n) - h(n')| \le 1 = c(n, n')$, ensuring $f(n)$ never decreases along any path.

## 2. Measured Results & Exploration Reductions
Evaluated on a $200 \times 200$ grid ($40{,}000$ cells) with 25% random obstacles:

| Pair Index | Start $\to$ Goal Coordinates | Optimal Path Length | Dijkstra Explored Nodes | A* (Manhattan) Explored | Exploration Reduction | Wall-Clock Speedup |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Pair 1** | $(10, 10) \to (190, 190)$ | 360 | 29,621 | 11,390 | **61.5%** | **2.13x** |
| **Pair 2** | $(10, 10) \to (35, 30)$ | 45 | 1,245 | 207 | **83.4%** | **7.02x** |
| **Pair 3** | $(20, 180) \to (180, 20)$ | 320 | 29,100 | 4,706 | **83.8%** | **5.28x** |
| **Pair 4** | $(100, 10) \to (100, 180)$ | 208 | 23,744 | 3,458 | **85.4%** | **6.53x** |
| **Pair 5** | $(15, 100) \to (185, 100)$ | 218 | 24,913 | 4,469 | **82.1%** | **5.74x** |

- **Average State Space Reduction**: **79.2% fewer states expanded**.
- **Path Length Guarantee**: In all 5 pairs, A* and Dijkstra produced identical path lengths.

## 3. Best vs Worst Case Behavior
- **Best Case (Open Grid / Direct Line of Sight)**: The Manhattan heuristic directs exploration strictly along the coordinate diagonal without spreading sideways. The number of explored cells approaches the path length $L$ ($O(L)$), achieving $> 85\%$ reduction compared to Dijkstra's radial expansion.
- **Worst Case (Concave Obstacles / U-Shaped Walls)**: When large obstacles block the line of sight, A* expends nodes exploring the dead-end before backtracking around the barrier.
- **Degenerate Heuristic ($h(n) = 0$)**:
  - Measured on Pair 2: Dijkstra explored **1,245 nodes** (Length 45); A* with $h=0$ explored **1,245 nodes** (Length 45).
  - When $h(n) = 0$, $f(n) = g(n) + 0 = g(n)$, making A* mathematically and operationally identical to Uniform-Cost Search (Dijkstra).

## 4. Inadmissible Heuristic Demonstration
An inadmissible heuristic overestimates the remaining cost ($h(n) > h^*(n)$).
- **Test Setup**: Heuristic inflated to $h(n) = 3.5 \times \text{Manhattan}(n)$.
- **Empirical Result**:
  - Admissible A* ($h = 1.0$): Path Length = **45**, Explored = 207.
  - Inadmissible A* ($h = 3.5$): Path Length = **59**, Explored = 73.
- **Takeaway**: While inflating $h(n)$ acts greedily to explore fewer nodes (73 vs 207), it **breaks the optimality guarantee**, returning a path that is **31.1% longer than optimal** ($59 > 45$) because the algorithm prematurely closed the true optimal corridor.

## 5. Memory Overhead Comparison
- **Dijkstra**: Requires distance array $g(n)$, predecessor array $p(n)$, and priority queue.
- **A***: Requires identical arrays ($g, p$) plus $f(n) = g(n) + h(n)$ computation and closed set booleans. The memory footprint is virtually identical ($O(V)$), but peak heap occupancy in A* is significantly smaller because far fewer nodes are enqueued before reaching the goal.
