# C-stuff
This repo consists of two projects (which don't really have anything in common lol)
- Word Searcher
- Augmented Data Structure to Improve Path Finding in large sparse graph

## WS
Here is a structured and comprehensive `README.md` generated based on your code and project description. It highlights the technical optimizations and provides clear usage instructions for your GitHub repository.

***

# Fast C Word Search & Indexer

A blazing-fast, lightweight command-line tool written in C that parses, indexes, and searches for words within large text documents. Built with a primary focus on minimizing execution time and optimizing memory footprint, this tool utilizes dynamic memory allocation and an efficient custom hash table to deliver sub-millisecond search queries.

## 🚀 Key Features

* **Lightning-Fast Indexing:** Uses a 1MB buffered read and `getc_unlocked()` to bypass standard I/O overhead, rapidly reading large files.
* **O(1) Search Complexity:** Implements a custom hash table utilizing the branchless, low-collision **FNV-1a** hashing algorithm.
* **Dynamic Memory Scaling:** Position arrays grow dynamically using a doubling strategy, achieving amortized $O(1)$ insertions while keeping memory usage tight.
* **Precise Occurrence Tracking:** Maps every instance of a word to its exact line and column number.
* **Smart Tokenization:** Automatically converts words to lowercase and preserves apostrophes (e.g., keeping "don't" as a single token) for intuitive searching.
* **Dual Execution Modes:** Search via direct command-line arguments or through an interactive REPL (Read-Eval-Print Loop) interface.
* **Built-in Profiling:** Displays precise time metrics for document indexing (in milliseconds) and query execution (in microseconds).

---

## 🛠️ Under the Hood: Performance Optimizations

This tool is explicitly designed for speed. Here is how it achieves it:

1. **FNV-1a Hash Function:** Chosen for its excellent dispersion properties and fast, bitwise operations, ensuring minimal collisions across the 65,536 buckets.
2. **Optimized I/O:** `getc_unlocked` avoids the locking overhead of standard thread-safe I/O functions. Combined with a custom `1 << 20` (1 MB) file buffer, disk reads are heavily minimized.
3. **Struct Packing & Chaining:** The `Entry` struct handles collisions via linked lists while keeping memory locality in mind for the dynamic `Position` arrays.

---

## 💻 Getting Started

### Prerequisites
* A standard C compiler (e.g., `gcc` or `clang`).
* A POSIX-compliant environment (Linux, macOS, or WSL on Windows) to support `getc_unlocked()`.

### Compilation

Clone the repository and compile the code using `gcc`:

```bash
gcc -O3 -o ws ws.c
```
*(Note: The `-O3` flag is highly recommended to enable compiler-level performance optimizations).*

---

## 📖 Usage

You can run the program in two ways: **Interactive Mode** or **CLI Argument Mode**.

### 1. Interactive Mode (REPL)
Pass only the document you want to index. The program will parse the file and open an interactive prompt.

```bash
./ws sample_document.txt
```

**Example Output:**
```text
Indexing "sample_document.txt" ...
Done. 4182 distinct words, 15023 total occurrences. [12.45 ms]

Enter a word to search (or "exit" to quit):
> algorithm
Search time: 1.25 µs
--------------------------------------------------------
  Word     : "algorithm"
  Found    : 3 occurrences
--------------------------------------------------------
  Line   42  |  col 15, 88
  Line  105  |  col 12
--------------------------------------------------------
```

### 2. CLI Argument Mode
Pass the document followed by a list of words you want to search for in a single execution. Ideal for scripting or quick lookups.

```bash
./ws sample_document.txt fast memory pointer
```

**Example Output:**
```text
Indexing "sample_document.txt" ...
Done. 4182 distinct words, 15023 total occurrences. [12.45 ms]

Search: "fast"  [0.85 µs]
--------------------------------------------------------
  Word     : "fast"
  Found    : 1 occurrence
--------------------------------------------------------
  Line   12  |  col 4
--------------------------------------------------------
...
```


||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\


## Augmented Data Structure
Here is a drafted `README.md` file for your GitHub repository based on your presentation and C++ implementation. 

***

### Path Finding Using Augmented Graphs

### Overview
[cite_start]This project implements an efficient path-finding algorithm using **Augmented Graphs**[cite: 5]. [cite_start]By identifying and leveraging highly connected nodes known as "hubs," the system accelerates complex network queries[cite: 48, 56]. [cite_start]Instead of relying entirely on traditional polynomial-time algorithms like BFS or Dijkstra for every query, this approach uses a one-time precomputation step to achieve near-constant or proportional query times[cite: 59, 226].

### Dataset
[cite_start]The implementation is designed to work with scale-free networks and is tested on the **Facebook Social Circles** dataset[cite: 29]. 
* [cite_start]**Nodes (Users):** 4,039 [cite: 31]
* [cite_start]**Edges (Friendships):** 88,234 [cite: 32]
* [cite_start]**Average Degree:** ~43.7 connections per user [cite: 33]
* [cite_start]**Structure:** Features a dense core of highly interconnected users with peripheral clusters, demonstrating the "small-world" effect[cite: 35, 36, 37]. 

*Note: The C++ program expects the input graph data in a file named `reduced_graph.csv`.*

### Core Concepts

### 1. Hub Calculation
* [cite_start]**Definition:** A hub is a highly connected node acting as a central point in the network[cite: 48].
* [cite_start]**Selection:** Nodes with a degree significantly higher than the network average are selected as hubs[cite: 49]. [cite_start]In the Facebook dataset, approximately 5% of nodes act as hubs but cover 90% of the shortest paths[cite: 251].

### 2. Precomputation Phase
[cite_start]The algorithm calculates and stores the shortest distances from every hub to all other reachable nodes in the graph[cite: 45]. 

### 3. Augmented Graph Structure
[cite_start]The augmented graph relies on three main data structures[cite: 66]:
* [cite_start]**Adjacency List:** Stores the base graph (nodes, neighbors, and edge weights)[cite: 67].
* [cite_start]**Hub Mapping:** Stores the precomputed distances from nodes to various hubs[cite: 67].
* [cite_start]**Hub Set:** Allows for $O(1)$ fast lookup of hub nodes[cite: 67, 216].

### Supported Queries
[cite_start]The augmented graph efficiently solves three main types of queries[cite: 66]:

1. **Reachability Query:**
   * **Mechanism:** Compares the hub sets of two nodes ($u$ and $v$). [cite_start]If they share any common hub, they are reachable[cite: 158, 159]. [cite_start]Falls back to checking direct edges if necessary[cite: 161].
2. **Shortest Path Query:**
   * [cite_start]**Mechanism:** Retrieves precomputed hub distances for nodes $u$ and $v$[cite: 184, 186]. [cite_start]It finds common hubs, sums the distances ($u \rightarrow hub \rightarrow v$), and selects the minimum sum[cite: 188, 192, 193].
3. **Path Query:**
   * [cite_start]**Mechanism:** Looks up a specific node in the hub mapping and retrieves an ordered list of all reachable hubs, sorted from nearest to farthest[cite: 213, 216, 219].

### Time Complexity Analysis
[cite_start]Trading a one-time precomputation cost for persistent efficiency gains drastically improves query speeds on large-scale networks[cite: 227, 228].

| Query Type | Augmented Graph | Traditional Method |
| :--- | :--- | :--- |
| **Precomputation** | $O(k(V+E))$ | N/A |
| **Reachability** | $O(1)$ | $O(V + E)$ (via BFS/DFS) |
| **Shortest Path** | $O(k)$ | $O(E + V \log V)$ (via Dijkstra) |
| **Path Query** | $O(k \log k)$ | $O(V + E)$ (via Traversal) |

[cite_start]*(Where $V$ = Vertices, $E$ = Edges, and $k$ = hub count per node, typically $k \ll V$)* [cite: 229, 230]

### Real-World Use Cases
[cite_start]This approach is highly applicable to modern network problems[cite: 231]:
* [cite_start]**Friend Recommendations:** Path queries can identify mutual hubs to suggest "Friends of Friends"[cite: 236].
* [cite_start]**Content Virality Prediction:** Shortest paths via hubs help model how information flows through influencers[cite: 240, 241].
* [cite_start]**Community Detection:** Analyzing hub neighborhoods reveals natural clusters like workplaces or schools[cite: 243].
* [cite_start]**Transport Networks:** Can be used to optimize routes using metro stations as connectivity anchors[cite: 246].


   ```
4. **Execute:**
   ```bash
   ./ADS
   ```
5. **Usage:** The program will parse the CSV, find hubs, and precompute distances. You will then be prompted to enter a source and destination node ID to find the shortest path.
