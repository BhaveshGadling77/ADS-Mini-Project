# Text Compression Algorithms & Data Structures

## 1. LZW (Lempel-Ziv-Welch) Compression

### Underlying Data Structure: Trie (Prefix Tree)
- **Concept:** Rather than storing dictionary strings in a hash table or flat array, the LZW encoder stores prefix sequences in an $N$-ary **Prefix Trie** (`adsc::LzwTrie`).
- **Nodes (`TrieNode`):**
  - `int32_t code`: The unique code assigned to this prefix.
  - `unordered_map<uint8_t, unique_ptr<TrieNode>> children`: Child byte transitions.
- **Lookup:** Advancing by one byte is an $O(1)$ child pointer traversal.
- **Insertion:** When a mismatch occurs on byte $c$ after node $P$, a new child node is added under $P$ with `code = nextCode++`. The search immediately restarts from the root's child for $c$.
- **Bit-Width Management:** Dynamically starts at 9 bits (values 0..511) and expands up to 16 bits (values 0..65535) as the dictionary fills, preventing wasteful bit consumption for small inputs.
- **Decompression:** Reconstructs the dictionary iteratively using a lookup table vector, correctly handling the classic $cScSc$ (code == dictionary size) edge case.

### Complexity
- **Time Complexity:** $O(N)$ for $N$ bytes of input.
- **Space Complexity:** $O(D)$ where $D \le 65536$ entries.

---

## 2. Static Huffman Coding

### Underlying Data Structures: Custom Min-Heap & Binary Tree
- **Pass 1 — Frequency Analysis:** Computes frequency counts for all 256 byte values in the input.
- **Pass 2 — Priority Queue Construction (`MinHeap`):**
  - All non-zero symbols are pushed into a custom template-based Min-Heap (`adsc::MinHeap<shared_ptr<HuffmanNode>>`).
  - Implements $O(\log K)$ `push()` (sift-up) and `pop()` (sift-down) operations.
- **Pass 3 — Huffman Tree Construction:**
  - Repeatedly pops the two lowest-frequency nodes ($L$ and $R$).
  - Merges them into a new internal node with `frequency = L->frequency + R->frequency`.
  - Pushes the combined node back into the heap until a single root node remains.
- **Pass 4 — Code Generation:**
  - Recursively traverses the binary tree to generate variable-length prefix bit strings (0 = left, 1 = right).
- **Pass 5 — Serialization & Decoding:**
  - Writes the non-zero symbol frequency table into the stream header.
  - Bit-packs encoded symbols using `BitWriter`.
  - The decompressor reconstructs the identical tree deterministically from frequencies, traversing from root to leaves with `BitReader`.

### Complexity
- **Time Complexity:** $O(N + K \log K)$ where $N$ is input length and $K \le 256$ is alphabet size.
- **Space Complexity:** $O(K)$ tree nodes.

---

## 3. Burrows-Wheeler Transform + Move-To-Front (BWT + MTF)

### Underlying Data Structures: Suffix / Rotation Array + Dynamic List
- **Stage 1 — Burrows-Wheeler Transform (BWT):**
  - Generates cyclic rotations of the input text and sorts them lexicographically.
  - Takes the last column $L$ of the sorted matrix and records the primary index $P$ (row of original text).
  - Groups identical characters together, creating long repetitive sequences.
- **Stage 2 — Move-To-Front (MTF) Transform:**
  - Maintains a dynamic 256-symbol alphabet list (`0..255`).
  - For each byte in $L$, finds its current rank/index, emits the rank, and moves the byte to the front of the list.
  - Transforms repetitive character clusters into long runs of zeroes ($0$).
- **Stage 3 — Zero Run-Length Encoding (RLE):**
  - Collapses runs of zeroes into `(0x00, run_count)` bytes.
- **Stage 4 — Inverse Reconstruction:**
  - Inverts RLE into MTF stream.
  - Inverts MTF using inverse list lookup.
  - Inverts BWT using LF-mapping vector $LF[i] = C[L[i]] + \text{rank}(L[i])$, tracing backwards from primary index $P$.

### Complexity
- **Time Complexity:** $O(N \log N)$ for rotation sorting and LF mapping.
- **Space Complexity:** $O(N)$ for index arrays.
