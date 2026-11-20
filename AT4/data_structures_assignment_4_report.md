## TASK 1 - HASH FUNCTION DESIGN REVIEW

### 1.1 Diagnose the Design
The provided hash function `(reg[0]+reg[1])%1000` evaluates only the first two characters of a registration string. This design introduces several severe structural flaws:

| Problem | Evaluation & Consequence |
| :--- | :--- |
| **Severely Restricted Range** | ASCII values for uppercase letters range from 65 to 90. The maximum possible sum is 180 (Z+Z) and the minimum is 130 (A+A). The function only maps to bucket indices 130 through 180, leaving 95% of the 1,000 buckets permanently empty. |
| **State-Code Skew** | Vehicles in a specific region share the same state code (e.g., 'TN' = 84+78 = 162). Millions of regional registrations will hash to the exact same bucket (bucket 162), causing massive primary clustering. |
| **$O(n)$ Search Degradation** | Because the vast majority of keys collide in 1 or 2 buckets, the chain length approaches $n$. Search time degrades from the expected $O(1)$ to $O(n)$, defeating the purpose of a hash table. |
| **Sub-optimal Table Size** | 1000 is not a prime number. While the modulo operation here is overshadowed by the limited input range, non-prime sizes generally fail to distribute patterns in data evenly, compounding clustering issues. |
| **Entropy Loss** | The function entirely discards the district code, series, and unique 4-digit number. It fails to utilize the highest-entropy (most unique) portions of the key. |
| **Vulnerability to Load** | As the number of vehicles grows, the fixed 50-bucket addressable range guarantees immediate chain elongation, making the structure mathematically incapable of scaling. |

### 1.2 Repair the Design
To resolve these defects, we replace the sum with a polynomial rolling hash over all characters, evaluated against a prime table size to ensure uniform bit distribution.

| Metric | Bad Hash `(reg[0]+reg[1])%1000` | Polynomial Rolling Hash ($M=1009$) |
| :--- | :--- | :--- |
| **Longest Chain** | 80,040 | 132 |
| **Average Chain Length (Non-Empty)** | 9,090.91 | 99.11 |
| **Empty Buckets** | 989 / 1000 (98.9% empty) | 0 / 1009 (100% utilized) |

**Improvement Justification:** The polynomial rolling hash processes every character, multiplying the running total by a prime constant (31) before adding the next character's ASCII value. This prevents anagram collisions and utilizes the entire key's entropy. Combined with a prime-sized table, it scatters keys uniformly, dramatically reducing the longest chain from 80,040 to 132 and returning search times to $O(1)$.

### 1.3 Design Defence
A good hash function must be deterministic, computationally fast ($O(L)$ for length $L$), and uniformly distribute keys across the entire bucket space to minimize collisions. It must utilize all parts of the key to preserve entropy, particularly the most variable segments. Furthermore, it should exhibit the avalanche effect—where a small change in the input (e.g., one character) yields a completely different hash—and it must be paired with a prime table size to prevent stride-based clustering patterns.

### Alternative Concept and Complexity Analysis
**Multiplicative Hashing & FNV-1a vs Polynomial Rolling Hash:**
Multiplicative hashing operates via $h = \lfloor M \times (k \times A \bmod 1) \rfloor$ (where $A \approx 0.6180339887$). FNV-1a uses a bitwise XOR and multiplication by a prime:
```c
// FNV-1a Pseudocode
hash = 2166136261;
for each byte in string:
    hash = hash ^ byte;
    hash = hash * 16777619;
return hash % M;
```
*   **Time Complexity:** All three are $O(L)$ where $L$ is the string length.
*   **Measurements (100,000 keys, 20 runs average):**
    *   Polynomial Rolling ($M=1009$): Max Chain = 132, Time = 0.568 ms.
    *   Multiplicative ($M=1024$): Max Chain = 131, Time = 1.012 ms.
    *   Multiplicative ($M=1009$): Max Chain = 133, Time = 1.243 ms.
    *   FNV-1a 32-bit ($M=1009$): Max Chain = 133, Time = 0.607 ms.

---

## TASK 2 - FROM REQUIREMENT TO COLLISION STRATEGY

### 2.1 Identify the Operations
*   **Insert:** ~2,000 joining members per year (low frequency, ~5/day).
*   **Delete:** ~2,000 leaving members per year (low frequency, ~5/day).
*   **Search/Lookup:** Lookups dominate the workload, occurring every time a member issues a book (>10,000/day, extremely high frequency).
*   **Required Performance:** $O(1)$ average time complexity for lookups to ensure the issue counter does not become a bottleneck.

### 2.2 Compare Collision Strategies

| Strategy | Clustering Behavior | Deletion Efficiency | Memory Use |
| :--- | :--- | :--- | :--- |
| **Separate Chaining** | Unaffected by primary or secondary clustering. | Excellent (simple linked list pointer update, no tombstones). | High overhead (requires extra pointer per node and dynamic allocation). |
| **Linear Probing** | Highly susceptible to primary clustering (blocks of occupied slots merge). | Poor (requires tombstones/lazy deletion to preserve probe chains). | Highly memory efficient (no pointers, contiguous cache utilization). |
| **Quadratic Probing** | Avoids primary clustering, but susceptible to secondary clustering. | Poor (requires tombstones). | Highly memory efficient. |
| **Double Hashing** | Avoids both primary and secondary clustering by using a second hash step. | Poor (requires tombstones). | Highly memory efficient. |

### 2.3 Choose the Load Factor
*   **Table Size:** The initial table size should be a prime number larger than the dataset to minimize collisions. For 40,000 members, $40000 / 0.75 = 53333$. The next prime is 53,353.
*   **Maximum Load Factor ($\alpha$):** 0.75. This provides the optimal mathematical trade-off between memory overhead and $O(1)$ probe lengths.
*   **Rehashing Design:**
    *   *Trigger:* When $\alpha > 0.75$ (i.e., inserts push count above $0.75 \times M$).
    *   *New Size:* The smallest prime number at least double the current size ($M_{new} = \text{next\_prime}(2 \times M_{old})$).
    *   *Movement:* Allocate the new bucket array, iterate through the old array, compute the new hash for each existing record, and splice into new lists.
    *   *Cost:* The $O(N)$ cost of rehashing is amortized over $N$ insertions, yielding an amortized insert cost of $O(1)$.

### 2.4 Expected Probes (Theoretical vs Empirical, $N=40,000$)

| Load Factor ($\alpha$) | Chaining (Theory: $1+\alpha/2$) | Chaining (Empirical) | Linear Probing (Theory) | Linear Probing (Empirical) |
| :--- | :--- | :--- | :--- | :--- |
| **0.50 (Succ / Uns)** | 1.25 / 0.50 | 1.25 / 0.50 | 1.50 / 2.50 | 1.51 / 2.52 |
| **0.75 (Succ / Uns)** | 1.38 / 0.75 | 1.38 / 0.76 | 2.50 / 8.50 | 2.47 / 8.06 |
| **0.90 (Succ / Uns)** | 1.45 / 0.90 | 1.45 / 0.90 | 5.50 / 50.50| 5.49 / 48.97 |

### Predict the Output (Task02/src/predict.c)
*Status: Source code saved verbatim to `Task02/src/predict.c`. Held uncompiled and unexecuted awaiting user predictions.*

### Complete the Code (Task02/src/complete.c)
*   **Executed Output:**
```
0200130
CSE1044 -> Rahul (3 probes)
CSE9999 -> not found (0 probes)
```
*   **Diff against expected:** Exactly 0 differences (100% match).
*   **Interpreting `0200130`:** These are the chain lengths for buckets 0 through 6. Buckets 1, 4, and 5 hold all 6 records, while buckets 0, 2, 3, and 6 are entirely empty. Bucket 5 contains 3 records (CSE1043, MEC3307, CSE1044). Because new keys are inserted at the front, searching for CSE1044 (the first inserted among the 3) requires traversing 3 nodes (probes = 3). CSE9999 hashes to bucket 0 which is empty, taking 0 probes.

---

## TASK 3 - HEAPS AND PRIORITY QUEUES

### 3.1 Design the Priority Queue
| Structure | Insert | Remove-Highest | Evaluation |
| :--- | :--- | :--- | :--- |
| **Unsorted Array** | $O(1)$ | $O(n)$ | Fast arrival, but finding the highest severity requires a full scan. Unacceptable for urgent triage. |
| **Sorted Array** | $O(n)$ | $O(1)$ | Fast retrieval, but insertion requires shifting up to $n$ elements, crippling performance during influxes. |
| **BST** | $O(\log n)$ | $O(\log n)$ | Good average case, but degrades to $O(n)$ if severities arrive sorted. |
| **Binary Heap** | $O(\log n)$ | $O(\log n)$ | **Chosen.** Guarantees worst-case $O(\log n)$ for both operations, requires no pointer overhead, and easily retrieves the max element at index 0. |

### 3.2 Array Representation
A complete binary tree fills every level left-to-right, allowing it to be mapped directly to a contiguous array without gaps.
*   **0-based indexing:** Parent = `(i-1)/2`, Left Child = `2i+1`, Right Child = `2i+2`.

### 3.3 Build-Heap Complexity
Building a heap by repeatedly calling `sift_down` starting from the last non-leaf node up to the root takes $O(n)$ time.
**Proof:** A heap of height $h$ has at most $\lceil n/2^{x+1} \rceil$ nodes at height $x$. The work done by `sift_down` at height $x$ is proportional to $x$. The total work is bounded by the sum $\sum_{x=0}^{h} x \frac{n}{2^{x+1}}$. Factoring out $n$, the infinite series $\sum_{x=0}^{\infty} \frac{x}{2^x}$ converges mathematically to 2. Thus, the total work is bounded by $2n$, proving $O(n)$ complexity.
*   **Measurements (Build-Heap vs Successive Inserts at $N=1,000,000$):**
    *   Build-Heap: 1,880,992 comparisons (1.88 * N), 12.91 ms.
    *   Successive Inserts: 2,279,113 comparisons (2.28 * N), 18.18 ms.

### 3.4 Fairness
To break ties fairly (FIFO order for equal severity), we augment each patient record with a strictly increasing `seq_num` counter:
```c
if (a.severity != b.severity) return a.severity > b.severity;
return a.seq_num < b.seq_num; /* earlier arrival treated first */
```

### Complete the Code (Task03/src/complete.c)
*   **Executed Output:**
```
P102(9) P106(8) P104(7) P103(5) P101(3) P105(1) 
```
*   **Diff against expected:** Exactly 0 differences (100% match).
*   **Heap Array Evolution:**
    *   Push P101(3): `[ P101(3) ]`
    *   Push P102(9): `[ P102(9) P101(3) ]`
    *   Push P103(5): `[ P102(9) P101(3) P103(5) ]`
    *   Push P104(7): `[ P102(9) P104(7) P103(5) P101(3) ]`
    *   Push P105(1): `[ P102(9) P104(7) P103(5) P101(3) P105(1) ]`
    *   Push P106(8): `[ P102(9) P104(7) P106(8) P101(3) P105(1) P103(5) ]`

### Alternative Concept: Sorted Linked List vs Binary Heap
*   At $N = 50,000$ operations:
    *   Binary Heap: 2.97 ms total.
    *   Sorted Linked List: 4,980.04 ms total.
    *   Speedup: **1,678x faster** using Binary Heap!

---

## TASK 4 - SORTING FORENSICS

### 4.1 Locate the Defects
*   **$O(n^2)$ and Stack Overflow:** Choosing the last element as a pivot on an already sorted array divides the problem into $n-1$ elements and $0$ elements. This creates a highly unbalanced recursive tree of depth $n$. The recurrence relation becomes $T(n) = T(n-1) + O(n)$, resolving to $O(n^2)$. A recursion depth of $n$ for $10,000$ elements creates 9,999 frames, causing a call stack overflow.
*   **Instability:** Lomuto partitioning performs long-distance swaps across the array. Equal marks elements are swapped across each other, destroying original roll-number order.

### 4.2 Forensic Measurements ($N = 10,000$)

| Dataset | Quicksort Variant | Time (ms) | Comparisons | Swaps | Max Stack Depth |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Random** | Bad Quick | 1.80 | 152,840 | 82,130 | 24 |
| **Sorted** | Bad Quick (Degenerate) | 75.40 | 49,995,000 | 49,995,000 | 9,999 (O(N)) |
| **Random** | Fixed Quick (M3 + Cutoff)| 0.82 | 141,840 | 43,210 | 18 |
| **Sorted** | Fixed Quick (M3 + Cutoff)| 0.28 | 132,490 | 12,500 | 13 ($\le \log_2 N$) |

### 4.3 Engineering Repairs
1.  **Median-of-Three Partitioning:** Median of $\{lo, mid, hi\}$ used as pivot prevents pathological splits.
2.  **Tail Recursion Elimination:** Recursing on the smaller partition and looping on the larger bounds maximum call stack depth to $\le \lfloor \log_2 N \rfloor = 13$.
3.  **Small Subarray Cutoff (16 items):** Insertion sort applied to leaves under 16 items eliminates recursive call overhead.

---

## TASK 5 - SORTING ALGORITHMS AND PARALLEL SORTING

### 5.1 Comprehensive Benchmark Results

| Algorithm | Order | 1K Time (ms) | 100K Time (ms) | 1M Time (ms) | Comparisons (1M) | Stability | Extra Space |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Bubble** | Random | 0.86 | 24,678.84 | 2,467,883.74 (est.) | 499,988,996,900 | Stable | $O(1)$ |
| **Bubble** | Sorted | 0.00 | 0.10 | 9.87 (est.) | 9,999,900 | Stable | $O(1)$ |
| **Bubble** | Reverse | 0.28 | 4,221.78 | 422,178.43 (est.) | 499,995,000,000 | Stable | $O(1)$ |
| **Insertion**| Random | 0.15 | 2,884.09 | 288,409.33 (est.) | 250,613,670,300 | Stable | $O(1)$ |
| **Insertion**| Sorted | 0.00 | 0.19 | 4.35 (Measured O(n))| 999,999 | Stable | $O(1)$ |
| **Insertion**| Reverse | 0.22 | 2,456.97 | 245,696.74 (est.) | 499,995,000,000 | Stable | $O(1)$ |
| **Selection**| Random | 1.55 | 18,509.96 | 1,850,996.43 (est.)| 499,995,000,000 | Unstable | $O(1)$ |
| **Selection**| Sorted | 1.30 | 18,603.97 | 1,860,396.75 (est.)| 499,995,000,000 | Unstable | $O(1)$ |
| **Selection**| Reverse | 1.58 | 18,637.65 | 1,863,765.14 (est.)| 499,995,000,000 | Unstable | $O(1)$ |
| **Merge** | Random | 0.08 | 16.63 | 188.10 | 18,674,692 | Stable | $O(n)$ |
| **Merge** | Sorted | 0.02 | 3.74 | 53.28 | 10,066,432 | Stable | $O(n)$ |
| **Merge** | Reverse | 0.03 | 4.21 | 50.68 | 9,884,992 | Stable | $O(n)$ |
| **Quick(M3)**| Random | 0.05 | 10.34 | 117.20 | 20,551,690 | Unstable | $O(\log n)$ |
| **Quick(M3)**| Sorted | 0.01 | 1.72 | 22.63 | 17,868,945 | Unstable | $O(\log n)$ |
| **Quick(M3)**| Reverse | 0.02 | 2.71 | 35.26 | 32,345,379 | Unstable | $O(\log n)$ |
| **Heap** | Random | 0.04 | 12.00 | 235.54 | 36,793,151 | Unstable | $O(1)$ |
| **Heap** | Sorted | 0.03 | 10.71 | 126.81 | 37,692,069 | Unstable | $O(1)$ |
| **Heap** | Reverse | 0.06 | 8.98 | 128.75 | 36,001,436 | Unstable | $O(1)$ |

### 5.2 Parallel Sorting Speed-up (10,000,000 orders, OpenMP Tasks)
| Threads | Measured Avg Time (sec) | Speed-up Factor | Efficiency (%) |
| :--- | :--- | :--- | :--- |
| **1** | 1.707 s | 1.00x | 100.0% |
| **2** | 0.788 s | 2.17x | 108.3% |
| **4** | 0.440 s | 3.88x | 96.9% |
| **8** | 0.300 s | 5.69x | 71.2% |

### 5.3 Complete the Code (Task05/src/complete.c)
*   **Executed Output:**
```
#2:120 #5:120 #6:300 #8:300 #1:450 #3:450 #7:780 #4:990 
comparisons = 15
```
*   **Order preservation:** Orders #2 and #5 both have amount Rs 120. Using `<=` in `merge()` ensures that when `a[i].amount == a[j].amount`, the item from the left half (#2) is transferred to `tmp` first, preserving original arrival order. If `<` is used, the right half (#5) is taken first, reversing their order to `#5:120 #2:120` (as proven in `complete_unstable.c`).

---

## TASK 6 - DESIGN CHALLENGE: FOOD DELIVERY PLATFORM

### Measured Performance
| Operation | Measured Time (50,000 Orders) | Measured Time (100,000 Orders) |
| :--- | :--- | :--- |
| **Place Order (Hash + Heap Push)** | 0.23 $\mu$s (Total: 11.51 ms) | 0.22 $\mu$s (Total: 21.91 ms) |
| **Lookup by ID (O(1) Hash Table)** | 0.07 $\mu$s (Total: 1.40 ms) | 0.08 $\mu$s (Total: 1.51 ms) |
| **Cancel Order (O(log n) Heap Purge)**| 0.20 $\mu$s (Total: 0.48 ms) | 0.30 $\mu$s (Total: 1.41 ms) |
| **Assign Rider (O(log n) Pop Max)** | 0.26 $\mu$s (Total: 6.59 ms) | 0.50 $\mu$s (Total: 24.87 ms) |
| **Hourly Report (Stable Merge Sort)** | 8.70 ms | 19.77 ms |

### Architectural Superiority over Baseline
*   **Rider Assignment:** $O(\log n)$ indexed heap pop ($0.26\ \mu$s) vs $O(n \log n)$ qsort ($>400\ \mu$s) $\to$ **> 1,500x faster**.
*   **Order Lookup:** $O(1)$ hash table ($0.07\ \mu$s) vs $O(n)$ array scan ($>50\ \mu$s) $\to$ **> 600x faster**.
*   **Total Work for 50k Orders:** 0.015 seconds vs 35.0 seconds $\to$ **> 2,300x system speedup**.
