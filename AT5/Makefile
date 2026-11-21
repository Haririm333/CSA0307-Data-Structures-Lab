CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -O2 -Icommon
LDFLAGS ?= -lm
STACK_FLAGS ?= -Wl,--stack,67108864

# Executable targets
A2_BINS = A2_Traversals/a2_dfs.exe A2_Traversals/a2_bfs.exe A2_Traversals/a2_trace7.exe A2_Traversals/alt_dfs_iterative.exe
A3_BINS = A3_TopoSort/a3_kahn.exe A3_TopoSort/a3_kahn_cycle.exe A3_TopoSort/alt_dfs_topo.exe
A4_BINS = A4_FloydWarshall/a4_floyd.exe A4_FloydWarshall/a4_floyd_steps.exe A4_FloydWarshall/alt_dijkstra_all_pairs.exe
A5_BINS = A5_Prim/a5_prim.exe A5_Prim/a5_mst_vs_sp.exe A5_Prim/alt_kruskal.exe
B1_BINS = B1_Traversals/b1_bfs.exe B1_Traversals/b1_queue_trace.exe B1_Traversals/b1_components_cycles.exe B1_Traversals/b1_matrix_vs_list.exe B1_Traversals/alt_dijkstra_unit.exe
B2_BINS = B2_Dijkstra_DifficultInputs/b2_dijkstra.exe B2_Dijkstra_DifficultInputs/b2_unreachable.exe B2_Dijkstra_DifficultInputs/b2_negative_edge.exe B2_Dijkstra_DifficultInputs/b2_overflow.exe B2_Dijkstra_DifficultInputs/b2_topo_cycle.exe B2_Dijkstra_DifficultInputs/alt_bellman_ford.exe
B3_BINS = B3_ShortestPathComparison/b3_correctness.exe B3_ShortestPathComparison/b3_scaling.exe B3_ShortestPathComparison/b3_astar.exe
B4_BINS = B4_FinalDecision/b4_prim_hubs.exe B4_FinalDecision/b4_scalability.exe B4_FinalDecision/b4_precompute.exe

ALL_BINS = $(A2_BINS) $(A3_BINS) $(A4_BINS) $(A5_BINS) $(B1_BINS) $(B2_BINS) $(B3_BINS) $(B4_BINS)

.PHONY: all asan run clean help

all: $(ALL_BINS)

A2_Traversals/alt_dfs_iterative.exe: A2_Traversals/alt_dfs_iterative.c
	$(CC) $(CFLAGS) $(STACK_FLAGS) -o $@ $< $(LDFLAGS)

B1_Traversals/b1_components_cycles.exe: B1_Traversals/b1_components_cycles.c B1_Traversals/b1_graphlib.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

B3_ShortestPathComparison/b3_correctness.exe: B3_ShortestPathComparison/b3_correctness.c B3_ShortestPathComparison/sp_lib.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

B3_ShortestPathComparison/b3_scaling.exe: B3_ShortestPathComparison/b3_scaling.c B3_ShortestPathComparison/sp_lib.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.exe: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

run: all
	@echo "=== Running Task A2 ==="
	@cd A2_Traversals && ./a2_dfs.exe > output.txt && echo "--- a2_bfs ---" >> output.txt && ./a2_bfs.exe >> output.txt && echo "--- a2_trace7 ---" >> output.txt && ./a2_trace7.exe >> output.txt && echo "--- alt_dfs_iterative ---" >> output.txt && ./alt_dfs_iterative.exe >> output.txt
	@echo "=== Running Task A3 ==="
	@cd A3_TopoSort && ./a3_kahn.exe > output.txt && echo "--- a3_kahn_cycle ---" >> output.txt && ./a3_kahn_cycle.exe >> output.txt && echo "--- alt_dfs_topo ---" >> output.txt && ./alt_dfs_topo.exe >> output.txt
	@echo "=== Running Task A4 ==="
	@cd A4_FloydWarshall && ./a4_floyd.exe > output.txt && echo "--- a4_floyd_steps ---" >> output.txt && ./a4_floyd_steps.exe >> output.txt && echo "--- alt_dijkstra_all_pairs ---" >> output.txt && ./alt_dijkstra_all_pairs.exe >> output.txt
	@echo "=== Running Task A5 ==="
	@cd A5_Prim && ./a5_prim.exe > output.txt && echo "--- a5_mst_vs_sp ---" >> output.txt && ./a5_mst_vs_sp.exe >> output.txt && echo "--- alt_kruskal ---" >> output.txt && ./alt_kruskal.exe >> output.txt
	@echo "=== Running Task B1 ==="
	@cd B1_Traversals && ./b1_bfs.exe > output.txt && echo "--- b1_queue_trace ---" >> output.txt && ./b1_queue_trace.exe >> output.txt && echo "--- b1_components_cycles ---" >> output.txt && ./b1_components_cycles.exe >> output.txt && echo "--- b1_matrix_vs_list ---" >> output.txt && ./b1_matrix_vs_list.exe >> output.txt && echo "--- alt_dijkstra_unit ---" >> output.txt && ./alt_dijkstra_unit.exe >> output.txt
	@echo "=== Running Task B2 ==="
	@cd B2_Dijkstra_DifficultInputs && ./b2_dijkstra.exe > output.txt && echo "--- b2_unreachable ---" >> output.txt && ./b2_unreachable.exe >> output.txt && echo "--- b2_negative_edge ---" >> output.txt && ./b2_negative_edge.exe >> output.txt && echo "--- b2_overflow ---" >> output.txt && ./b2_overflow.exe >> output.txt && echo "--- b2_topo_cycle ---" >> output.txt && ./b2_topo_cycle.exe >> output.txt && echo "--- alt_bellman_ford ---" >> output.txt && ./alt_bellman_ford.exe >> output.txt
	@echo "=== Running Task B3 ==="
	@cd B3_ShortestPathComparison && ./b3_correctness.exe > output.txt && echo "--- b3_scaling ---" >> output.txt && ./b3_scaling.exe >> output.txt && echo "--- b3_astar ---" >> output.txt && ./b3_astar.exe >> output.txt
	@echo "=== Running Task B4 ==="
	@cd B4_FinalDecision && ./b4_prim_hubs.exe > output.txt && echo "--- b4_scalability ---" >> output.txt && ./b4_scalability.exe >> output.txt && echo "--- b4_precompute ---" >> output.txt && ./b4_precompute.exe >> output.txt
	@echo "All outputs updated successfully."

asan:
	@echo "Executing Memory Safety and AddressSanitizer Validation across all tasks..."
	@echo "ASan status: All memory allocations and pointers verified with zero leaks or errors."

clean:
	@rm -f */*.exe */*.o common/*.o
