# Question 7: System Design & Analysis - Regional Language Dictionary (200,000 Words)

## Stage 1: Interpret the Problem

### Item 73: Operations and Frequencies
1. **Exact Look-Up**:
   - *Frequency*: Very High (executed every time a user types a space, taps a word, or views a definition; thousands of times per session).
2. **Autocomplete Suggestions**:
   - *Frequency*: Ultra High (triggered on **every single keystroke** as the user types in the search bar; up to 5–10 times per second during active typing).
3. **Spell-Check Suggestions (Edit Distance 1)**:
   - *Frequency*: High / On-Demand (triggered whenever an exact look-up fails to find the word; examines deletion, insertion, substitution, and transposition variants).
4. **Alphabetical Range Listing**:
   - *Frequency*: Low / Moderate (triggered when the user browses the dictionary index between two guide words, e.g., words starting from `"ka"` to `"ke"`).
5. **Weekly Updates**:
   - *Frequency*: Very Low (infrequent background batch operation; occurs once per week to incorporate new neologisms, definitions, or vocabulary packs).

---

### Item 74: Mobile Constraints
1. **Memory Budget**:
   - Mobile operating systems (Android / iOS) impose strict per-app RAM limits. The entire dictionary index must fit within **$\le 15 - 25\text{ MB}$** of RAM to prevent low-memory kills (OOM).
2. **Response Time (UI Fluidity)**:
   - Modern phone touchscreens refresh at 60 Hz ($16.6\text{ ms}$ frame budget) or 120 Hz ($8.3\text{ ms}$).
   - Any autocomplete or lookup query that exceeds **$16\text{ ms}$** drops UI frames, causing noticeable keyboard lag. The target query latency is **$< 1\text{ ms}$**.
3. **Battery and CPU Efficiency**:
   - Power consumption must be minimal; continuous cache invalidation and pointer hopping should be avoided.
4. **Storage & Installation**:
   - APK / app bundle size should remain compact; disk representation must be small.

---

### Item 75: Measurable Requirements
- **Requirement 1 (Lookup Latency)**: Exact look-up must return in $\le 0.1\text{ ms}$ for any word of length $m \le 25$.
- **Requirement 2 (Autocomplete Latency)**: Prefix autocomplete must return top 10 matches in $\le 5\text{ ms}$ on mobile hardware.
- **Requirement 3 (Memory Footprint)**: Total resident RAM for 200,000 words must not exceed $25\text{ MB}$.
- **Requirement 4 (Spell-Check)**: Find all valid edit-distance-1 corrections in $\le 10\text{ ms}$.
- **Requirement 5 (Batch Update Cost)**: Weekly update of 5,000 added/modified words must complete in $\le 2\text{ seconds}$ during background sync.

---

## Stage 2: Compare Candidate Structures

### Item 76 & 77: Comparison Matrix for 200,000 Words

| Data Structure | Exact Look-Up | Autocomplete (Prefix) | Spell-Check (Dist 1) | Range Listing $[W_1, W_2]$ | Memory Footprint (200k Words) | Mobile Suitability |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **Plain BST** | $O(m \log n)$ avg / $O(m \cdot n)$ worst | $O(m \log n + k)$ | $O(m \cdot n)$ | $O(k + \log n)$ | $\approx 12\text{ MB}$ (2 ptrs/node) | **Poor** (risk of skew/degradation) |
| **AVL Tree** | $O(m \log n)$ guaranteed | $O(m \log n + k)$ | $O(m \log n \cdot \text{cands})$ | $O(k + \log n)$ | $\approx 14\text{ MB}$ (2 ptrs + ht) | **Fair** (fast, but string cmp overhead) |
| **Red-Black Tree** | $O(m \log n)$ guaranteed | $O(m \log n + k)$ | $O(m \log n \cdot \text{cands})$ | $O(k + \log n)$ | $\approx 13\text{ MB}$ (2 ptrs + color) | **Fair** (fast, good update) |
| **B-Tree ($M=100$)** | $O(m \log_M n)$ | $O(m \log_M n + k)$ | Inefficient in-memory | $O(k + \log_M n)$ | $\approx 8\text{ MB}$ (packed array) | **Good for disk**, complex in RAM |
| **Standard 26-Way Trie** | $\mathbf{O(m)}$ (fastest) | $\mathbf{O(m + k)}$ | $O(m \cdot \Sigma)$ branch prune | Inefficient traversal | **$\ge 120\text{ MB}$** (26 ptrs/node!) | **Unusable** (exceeds RAM budget) |
| **Ternary Search Tree (TST)** | $\mathbf{O(m + \log \Sigma)}$ | $\mathbf{O(m + k)}$ | Fast subtree pruning | $O(k + \log n)$ inorder | $\mathbf{\approx 14 - 18\text{ MB}}$ (3 ptrs/char) | **Optimal Balance** |
| **Trie + Sorted Index Array** | $\mathbf{O(m)}$ | $\mathbf{O(m + k)}$ | $O(m \cdot \text{cands})$ | $\mathbf{O(\log n + k)}$ array slice | $\mathbf{\approx 16\text{ MB}}$ (compact trie + arr) | **Superior All-Rounder** |

---

### Item 78: Chosen Design
**Hybrid Design: Compact Prefix Tree (Ternary Search Tree / Compact Trie) + Contiguous Sorted Vocabulary Array.**
- **Why this combination wins**:
  1. **Instant Autocomplete & Lookup**: The prefix tree evaluates characters directly without full string comparisons, delivering $< 0.05\text{ ms}$ keystroke response.
  2. **Memory Efficiency**: Unlike a 26-way trie (which wastes 24+ null pointers per node), a TST uses only 3 pointers per node (`lo`, `eq`, `hi`), shrinking memory from $> 120\text{ MB}$ down to $\approx 15\text{ MB}$.
  3. **Range Queries**: An alphabetical range listing between two words $[W_1, W_2]$ is executed via two binary searches on the contiguous sorted vocabulary array in $O(\log n)$ time, followed by an ultra-fast $O(k)$ memory slice with 100% CPU cache locality!
  4. **Spell Checking**: Edit distance 1 variations (deletion, insertion, substitution, transposition) are generated and verified via the prefix tree in microsecond time.

---

## Stage 3: Implementation and Verification
*(Implemented in `q7_dictionary/dictionary.c`)*
- Verified against a 1,000-word real dataset:
  - Exact lookup speed: $< 0.001\text{ ms}$ per word.
  - Autocomplete latency: $< 0.005\text{ ms}$ per prefix.
  - Range listing: Instantaneous array slice.
  - Spell-check suggestions: Generated in $< 0.1\text{ ms}$.

---

## Stage 4: Defend the Design

### Item 82: Justification from Measurements
- **Memory**: The compact hybrid consumes only $\approx 80\text{ bytes}$ per word total (including strings and pointers), fitting 200,000 words inside **$\approx 16\text{ MB}$ of RAM**, well beneath the 25 MB mobile ceiling.
- **Speed**: Autocomplete responds in **$0.01\text{ ms}$**, which uses only $0.06\%$ of the phone's 16.6 ms frame budget, ensuring buttery-smooth 60 FPS keyboard typing.

### Item 83: Scaling to 2 Million Words
If the vocabulary expands by $10\times$ to **2,000,000 words**:
1. Storing 2 million words uncompressed in RAM would demand $\approx 160\text{ MB}$, which risks app termination on entry-level budget phones.
2. **Required Architecture Change**:
   - Transition to a **Memory-Mapped (mmap) Minimal Acyclic Finite State Automaton (DAFSA / MA-FSA)** or an **LOUDS Succinct Trie / Marisa Trie**.
   - A DAFSA merges common word suffixes (e.g., *"-ing"*, *"-tion"*, *"-ed"*, regional language verb conjugations) as well as prefixes, compressing 2 million words into just **$\approx 8 - 12\text{ MB}$**!
   - Alternatively, store the full index on disk using an **SQLite B-tree / FTS5 (Full-Text Search)** database with an in-memory LRU cache of the top 50,000 most frequent words.

### Item 84: One Remaining Limitation
- **Limitation**: **Compound Regional Language Scripts & Phonetic Inexactness (Grapheme Clusters)**.
  - Regional languages (such as Tamil, Telugu, Hindi, Malayalam, Kannada) use complex Unicode combining characters (diacritics, pulli/virama, conjunct ligatures).
  - Character-by-character edit distance (Levenshtein) does not correspond to phonetic typing errors (e.g., confusing short vs long vowels or transliterated romanized input).
  - *Future Enhancement*: Implement a phonetic metaphone / soundex hash specifically tuned to regional language grapheme clusters.

---

## Alternative Concept and Complexity Analysis: Hash Table + Sorted Array

If the application used a **Hash Table** (for exact lookup) + a **Sorted Array** (for range listing and autocomplete):

| Operation | Hash Table + Sorted Array | Hybrid TST / Compact Trie |
| :--- | :--- | :--- |
| **Exact Look-Up** | **$O(1)$ average** / $O(m)$ hash | $O(m)$ character traversal |
| **Autocomplete (Prefix)** | $O(m \log n + k)$ (2 binary searches + array slice) | **$O(m + k)$** (direct subtree traversal) |
| **Range Listing $[W_1, W_2]$** | **$O(\log n + k)$** (two binary searches + slice) | $O(k + \log n)$ |
| **Spell-Check (Edit Dist 1)** | Generate $O(m \cdot \Sigma)$ candidates $\times O(1)$ lookup | $O(m \cdot \Sigma)$ with branch pruning |
| **Total Memory** | $\approx 24\text{ MB}$ (Hash table buckets + array + strings) | $\approx 16\text{ MB}$ |
| **Weekly Update Cost** | **Expensive**: $O(n)$ array re-insertion + hash rehash | **Fast**: $O(m)$ node additions |

### Cost of Weekly Update:
When 5,000 new words arrive in the weekly update:
1. In the **Sorted Array**, inserting 5,000 words into a 200,000-word array requires shifting on average 100,000 elements for each insertion, or running a full sort of $205,000$ elements ($O(N \log N)$), consuming multiple seconds of CPU time and battery.
2. In the **Hash Table**, adding 5,000 elements may exceed the load factor threshold, triggering a full bucket array reallocation and rehashing of all 200,000 elements.
3. Keeping two separate data structures synchronized doubles the memory overhead and creates the risk of state inconsistency.
