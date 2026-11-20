# Assignment 4 (CO4) - Consolidated Screenshots Directory

This directory contains all the terminal execution outputs, benchmark comparison graphs, and verification runs across Tasks 01 through 06.

---

### Screenshot Catalog

| File Name | Task | Content Description |
| :--- | :--- | :--- |
| `Task01_01_eval_terminal.png` | Task 01 | Terminal execution of `generate_input` and `hash_eval` on 100,000 keys |
| `Task01_02_chain_distribution_plot.png` | Task 01 | Matplotlib plot comparing Bad Hash vs Polynomial Rolling Hash distribution |
| `Task02_01_complete_probes_rehash_terminal.png` | Task 02 | Terminal output for `complete.c` (0 diff), theoretical vs empirical probes table, and dynamic rehashing demo |
| `Task03_01_complete_fair_terminal.png` | Task 03 | Terminal output for `complete.c` (0 diff) and `fair_heap.c` FIFO demonstration |
| `Task03_02_buildheap_bench_plot.png` | Task 03 | Matplotlib plot showing bottom-up Build-Heap ($O(n)$) vs top-down successive inserts ($O(n \log n)$) |
| `Task03_03_pq_comparison_plot.png` | Task 03 | Matplotlib plot comparing Binary Heap vs Sorted Linked List Priority Queue (>1,600x speedup) |
| `Task04_01_sorting_forensics_terminal.png` | Task 04 | Terminal output showing Bad Quicksort $O(n^2)$ degradation (49.99M comps, 9,999 depth) vs Repaired Quicksort and stability proof |
| `Task05_01_complete_parallel_merge_terminal.png` | Task 05 | Terminal output for `complete.c` (0 diff), stability inversion test with `<`, and OpenMP parallel merge sort on 10M orders |
| `Task05_02_parallel_speedup_plot.png` | Task 05 | Matplotlib plot showing parallel merge sort speedup vs ideal linear scaling |
| `Task06_01_tests_and_simulator_terminal.png` | Task 06 | Terminal output for `test_system.c` (all asserts passing), 100,000 orders simulator, and baseline comparison |
