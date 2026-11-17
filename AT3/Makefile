CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
OPTFLAGS = -Wall -Wextra -std=c11 -g -O2
ASAN_FLAGS = -Wall -Wextra -std=c11 -g -fsanitize=address,undefined

# Directories
Q1_OUT = q1_bst/output
Q2_OUT = q2_avl/output
Q3_OUT = q3_btree/output
Q4_OUT = q4_rbtree/output
Q5_OUT = q5_splay/output
Q6_OUT = q6_trie/output
Q7_OUT = q7_dictionary/output

.PHONY: all clean run asan q1 q2 q3 q4 q5 q6 q7 run_q1 run_q2 run_q3 run_q4 run_q5 run_q6 run_q7 screenshots

all: q1 q2 q3 q4 q5 q6 q7

# --- Question 1: BST ---
q1: q1_bst/bst.exe q1_bst/q1_predict1.exe q1_bst/q1_predict2.exe q1_bst/book.exe

q1_bst/bst.exe: q1_bst/bst.c
	$(CC) $(CFLAGS) $< -o $@

q1_bst/q1_predict1.exe: q1_bst/q1_predict1.c
	$(CC) $(CFLAGS) $< -o $@

q1_bst/q1_predict2.exe: q1_bst/q1_predict2.c
	$(CC) $(CFLAGS) $< -o $@

q1_bst/book.exe: q1_bst/book.c
	$(CC) $(CFLAGS) $< -o $@

run_q1: q1
	@mkdir -p $(Q1_OUT)
	./q1_bst/book.exe | tee $(Q1_OUT)/book_output.txt
	./q1_bst/q1_predict1.exe | tee $(Q1_OUT)/predict1_output.txt
	./q1_bst/q1_predict2.exe | tee $(Q1_OUT)/predict2_output.txt
	./q1_bst/bst.exe | tee $(Q1_OUT)/bst_output.txt

# --- Question 2: AVL ---
q2: q2_avl/stock_avl.exe q2_avl/avl.exe

q2_avl/stock_avl.exe: q2_avl/stock_avl.c
	$(CC) $(CFLAGS) $< -o $@

q2_avl/avl.exe: q2_avl/avl.c
	$(CC) $(OPTFLAGS) $< -o $@

run_q2: q2
	@mkdir -p $(Q2_OUT)
	./q2_avl/stock_avl.exe | tee $(Q2_OUT)/stock_avl_output.txt
	./q2_avl/avl.exe | tee $(Q2_OUT)/avl_output.txt

# --- Question 3: B-Tree ---
q3: q3_btree/btree.exe

q3_btree/btree.exe: q3_btree/btree.c
	$(CC) $(CFLAGS) $< -o $@

run_q3: q3
	@mkdir -p $(Q3_OUT)
	./q3_btree/btree.exe | tee $(Q3_OUT)/btree_output.txt

# --- Question 4: Red-Black Tree ---
q4: q4_rbtree/rbtree.exe

q4_rbtree/rbtree.exe: q4_rbtree/rbtree.c
	$(CC) $(CFLAGS) $< -o $@

run_q4: q4
	@mkdir -p $(Q4_OUT)
	./q4_rbtree/rbtree.exe | tee $(Q4_OUT)/rbtree_output.txt

# --- Question 5: Splay Tree ---
q5: q5_splay/splay.exe

q5_splay/splay.exe: q5_splay/splay.c
	$(CC) $(CFLAGS) $< -o $@

run_q5: q5
	@mkdir -p $(Q5_OUT)
	./q5_splay/splay.exe | tee $(Q5_OUT)/splay_output.txt

# --- Question 6: Trie ---
q6: q6_trie/complete_trie.exe q6_trie/trie.exe

q6_trie/complete_trie.exe: q6_trie/complete_trie.c
	$(CC) $(CFLAGS) $< -o $@

q6_trie/trie.exe: q6_trie/trie.c
	$(CC) $(CFLAGS) $< -o $@

run_q6: q6
	@mkdir -p $(Q6_OUT)
	./q6_trie/complete_trie.exe | tee $(Q6_OUT)/complete_trie_output.txt
	./q6_trie/trie.exe | tee $(Q6_OUT)/trie_output.txt

# --- Question 7: Dictionary ---
q7: q7_dictionary/dictionary.exe

q7_dictionary/dictionary.exe: q7_dictionary/dictionary.c
	$(CC) $(CFLAGS) $< -o $@

run_q7: q7
	@mkdir -p $(Q7_OUT)
	./q7_dictionary/dictionary.exe | tee $(Q7_OUT)/dictionary_output.txt

# --- Run All & Screenshots ---
run: run_q1 run_q2 run_q3 run_q4 run_q5 run_q6 run_q7 screenshots

screenshots:
	python generate_screenshots.py

asan:
	$(CC) $(ASAN_FLAGS) q1_bst/bst.c -o q1_bst/bst_asan.exe || true
	$(CC) $(ASAN_FLAGS) q6_trie/complete_trie.c -o q6_trie/complete_trie_asan.exe || true
	$(CC) $(ASAN_FLAGS) q6_trie/trie.c -o q6_trie/trie_asan.exe || true

clean:
	rm -f q1_bst/*.exe q2_avl/*.exe q3_btree/*.exe q4_rbtree/*.exe q5_splay/*.exe q6_trie/*.exe q7_dictionary/*.exe
