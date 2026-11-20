# Data Structures Assignment 4 (CO4)
## Designing Fast Storage and Ordering of Data

**Repository:** `CSA03-<REGNO>-Assignment4`  
**Student Name:** Student  
**Register Number:** `<REGNO>`  
**Compiler:** GCC 16.2.0 (MSYS2 UCRT64 / MinGW-w64)  
**Standard & Compilation Flags:** `gcc -std=c11 -Wall -Wextra` (with `-O2` for benchmarks, `-fsanitize=address` for complete-the-code, `-fopenmp` for parallel sort)  

---

## Overview & Directory Architecture

This repository contains the complete, rigorous implementation of all six K5-level evaluation tasks specified in the Data Structures Assignment 4 (CO4) specification. Every task is organized in an independent folder (`Task01` through `Task06`) containing `src/`, `input/`, and `output/` subdirectories.

```
CSA03-<REGNO>-Assignment4/
├── README.md                                 # Root documentation & execution instructions
├── data_structures_assignment_4_report.md    # Comprehensive academic report with all evaluations
├── Task01/                                   # Task 1: Hash Function Design Review
│   ├── src/ (generate_input.c, hash_eval.c, plot_distributions.py)
│   ├── input/ (registrations.txt - 100,000 keys)
│   └── output/ (hash_comparison.csv, chain_distribution.csv, chain_distribution.png, results.txt)
├── Task02/                                   # Task 2: Collision Strategy & Library Member Lookup
│   ├── src/ (predict.c, complete.c, linear_probing.c, probes_table.c, rehash_demo.c)
│   ├── input/
│   └── output/ (complete_output.txt, expected_output.txt, diff.txt, probes_table.csv, rehash_log.txt, results.txt)
├── Task03/                                   # Task 3: Heaps & Priority Queues (Emergency Triage)
│   ├── src/ (predict.c, complete.c, complete_log.c, buildheap_bench.c, fair_heap.c, sorted_list_pq.c, plot_*.py)
│   ├── input/
│   └── output/ (complete_output.txt, expected_output.txt, diff.txt, buildheap_bench.png, pq_comparison.png, results.txt)
├── Task04/                                   # Task 4: Sorting Forensics & Quicksort Repair
│   ├── src/ (predict.c, quick_bad.c, quick_fixed.c, stable_demo.c)
│   ├── input/
│   └── output/ (quick_bad_results.txt, quick_fixed_results.txt, stability_demo.txt, comparison_table.csv, results.txt)
├── Task05/                                   # Task 5: Sorting Benchmarks & Parallel Merge Sort
│   ├── src/ (complete.c, complete_unstable.c, sorts_bench.c, parallel_merge.c, plot_speedup.py)
│   ├── input/
│   └── output/ (complete_output.txt, diff.txt, sorts_bench.csv, parallel_speedup.csv, parallel_speedup.png, results.txt)
└── Task06/                                   # Task 6: Design Challenge - Food Delivery Platform
    ├── src/ (order_system.h, order_system.c, simulator.c, baseline_system.c, test_system.c)
    ├── input/
    └── output/ (simulator_metrics.csv, baseline_comparison.txt, results.txt)
```

---

## Predict the Output Status

As mandated by global instructions, the following three programs have their source code saved **verbatim** from the specification, but have **NOT** been compiled or executed. They are awaiting your written predictions:

1. `Task02/src/predict.c` (Page 4: Linear probing array simulation with `{42, 17, 52, 27, 99, 62}`)
2. `Task03/src/predict.c` (Page 8: Min-heap construction via `sift_down` on `{9, 4, 7, 1, 8, 2, 6}`)
3. `Task04/src/predict.c` (Page 12: Lomuto partitioning tracing on `{7, 2, 9, 4, 3, 8, 5}`)

Once your predictions are documented, you can run them via the commands listed below.

---

## Tasks Summary & Exact Commands

### TASK 01: Hash Function Design Review
- **Exact Compile Command:**
  ```bash
  gcc -std=c11 -Wall -Wextra -O2 Task01/src/generate_input.c -o Task01/generate_input
  gcc -std=c11 -Wall -Wextra -O2 Task01/src/hash_eval.c -o Task01/hash_eval
  ```
- **Execution Steps:**
  ```bash
  ./Task01/generate_input
  ./Task01/hash_eval
  python Task01/src/plot_distributions.py
  ```
- **Results Note:** The bad hash function `(reg[0]+reg[1])%1000` mapped all 80,000 TN vehicles to bucket 162, leaving 989 buckets completely empty and degrading lookup to $O(n)$ ($80,040$ max chain). The polynomial rolling hash ($M=1009$) achieved 100% bucket utilization with a max chain of only 132 in $0.57$ ms.

---

### TASK 02: Collision Strategies & Library Member Lookup
- **Exact Compile Command:**
  ```bash
  gcc -std=c11 -Wall -Wextra Task02/src/complete.c -o Task02/complete
  gcc -std=c11 -Wall -Wextra Task02/src/linear_probing.c -o Task02/linear_probing
  gcc -std=c11 -Wall -Wextra -O2 Task02/src/probes_table.c -o Task02/probes_table
  gcc -std=c11 -Wall -Wextra Task02/src/rehash_demo.c -o Task02/rehash_demo
  ```
- **Execution Steps:**
  ```bash
  ./Task02/complete
  ./Task02/linear_probing
  ./Task02/probes_table
  ./Task02/rehash_demo
  ```
- **Results Note:** `complete.c` produced exact output `0200130 / CSE1044 -> Rahul (3 probes) / CSE9999 -> not found (0 probes)` matching the specification with 0 diff. Empirical probe measurements on 40,000 keys verified that at $\alpha=0.90$, linear probing unsuccessful probes explode to $48.97$ due to primary clustering, while separate chaining stays flat at $0.90$ probes.

---

### TASK 03: Heaps & Priority Queues (Emergency Triage)
- **Exact Compile Command:**
  ```bash
  gcc -std=c11 -Wall -Wextra Task03/src/complete.c -o Task03/complete
  gcc -std=c11 -Wall -Wextra Task03/src/complete_log.c -o Task03/complete_log
  gcc -std=c11 -Wall -Wextra -O2 Task03/src/buildheap_bench.c -o Task03/buildheap_bench
  gcc -std=c11 -Wall -Wextra Task03/src/fair_heap.c -o Task03/fair_heap
  gcc -std=c11 -Wall -Wextra -O2 Task03/src/sorted_list_pq.c -o Task03/sorted_list_pq
  ```
- **Execution Steps:**
  ```bash
  ./Task03/complete
  ./Task03/complete_log
  ./Task03/buildheap_bench
  python Task03/src/plot_buildheap.py
  ./Task03/fair_heap
  ./Task03/sorted_list_pq
  python Task03/src/plot_pq.py
  ```
- **Results Note:** `complete.c` produced `P102(9) P106(8) P104(7) P103(5) P101(3) P105(1)` with 0 diff. Bottom-up build-heap was mathematically proven and measured to take linear comparisons ($\sim 1.88n$), while sorted linked-list priority queues degraded quadratically, running over 1,600x slower than the binary heap at 50,000 operations.

---

### TASK 04: Sorting Forensics & Quicksort Repair
- **Exact Compile Command:**
  ```bash
  gcc -std=c11 -Wall -Wextra -Wl,--stack,67108864 Task04/src/quick_bad.c -o Task04/quick_bad
  gcc -std=c11 -Wall -Wextra -O2 Task04/src/quick_fixed.c -o Task04/quick_fixed
  gcc -std=c11 -Wall -Wextra Task04/src/stable_demo.c -o Task04/stable_demo
  ```
- **Execution Steps:**
  ```bash
  ./Task04/quick_bad
  ./Task04/quick_fixed
  ./Task04/stable_demo
  ```
- **Results Note:** Bad quicksort on sorted data triggered $O(n^2)$ comparisons ($49,995,000$) and reached 9,999 recursion frames (severe stack overflow risk). The repaired quicksort with median-of-three pivot and tail-recursion elimination bounded stack depth to 13 frames ($\le \log_2 n$) and reduced runtime from $75.40$ ms to $0.28$ ms, while `stable_demo` confirmed stability preservation under merge and counting sort.

---

### TASK 05: Sorting Benchmarks & Parallel Merge Sort
- **Exact Compile Command:**
  ```bash
  gcc -std=c11 -Wall -Wextra Task05/src/complete.c -o Task05/complete
  gcc -std=c11 -Wall -Wextra Task05/src/complete_unstable.c -o Task05/complete_unstable
  gcc -std=c11 -Wall -Wextra -O2 Task05/src/sorts_bench.c -o Task05/sorts_bench
  gcc -std=c11 -Wall -Wextra -O2 -fopenmp Task05/src/parallel_merge.c -o Task05/parallel_merge
  ```
- **Execution Steps:**
  ```bash
  ./Task05/complete
  ./Task05/complete_unstable
  ./Task05/sorts_bench
  ./Task05/parallel_merge
  python Task05/src/plot_speedup.py
  ```
- **Results Note:** `complete.c` verified stability preservation using `<=`, while `<` inverted equal elements `#2` and `#5`. Benchmark confirmed quadratic sorts take $>2,400$ seconds at 1M elements, while parallel merge sort scaled to 10,000,000 elements across 8 threads, achieving a 5.69x speedup ($0.300$ s vs $1.707$ s single-threaded).

---

### TASK 06: Design Challenge - Food Delivery Platform
- **Exact Compile Command:**
  ```bash
  gcc -std=c11 -Wall -Wextra -O2 Task06/src/order_system.c Task06/src/simulator.c -o Task06/simulator
  gcc -std=c11 -Wall -Wextra -O2 Task06/src/order_system.c Task06/src/baseline_system.c -o Task06/baseline_system
  gcc -std=c11 -Wall -Wextra Task06/src/order_system.c Task06/src/test_system.c -o Task06/test_system
  ```
- **Execution Steps:**
  ```bash
  ./Task06/test_system
  ./Task06/simulator
  ./Task06/baseline_system
  ```
- **Results Note:** The dual hash table + indexed max-heap architecture achieved $0.08\ \mu$s lookups, $0.30\ \mu$s cancellations, and $0.50\ \mu$s rider dispatches at 100,000 orders. Against a naive array re-sorting baseline, the designed system delivered a $>2,300$x total speedup for 50,000 orders ($0.015$ s vs $35.0$ s), passing all unit test assertions.
