## High-Performance Algorithmic Solvers

## Overview
A collection of highly optimized algorithmic solutions and custom data structures implemented in C/C++. This repository demonstrates proficiency in low-level memory management, algorithm design (graph traversal, balanced trees), and building scalable applications without heavy reliance on standard libraries (STL).

## Included Implementations

### 1. AVL Interval Tree with Lazy Propagation (`avl-interval-tree`)
*   **Concepts:** Self-balancing Binary Search Trees (AVL), Lazy Propagation, Manual Memory Management.
*   **Description:** A complex implementation of an AVL tree that supports efficient range updates (enchanting a range of elements) using lazy propagation. Nodes allocate memory dynamically and balance themselves via custom rotation algorithms to guarantee $O(\log N)$ time complexity for insertions, deletions, and queries.

### 2. Custom Hash Table (`custom-hash-table`)
*   **Concepts:** Hash Maps, Collision Resolution (Chaining), The Rule of Three (Copy Constructor, Assignment Operator, Destructor).
*   **Description:** A fully custom implementation of a Hash Table built on raw arrays of pointers. It handles collisions via linked lists and implements deep copying and safe memory deallocation, ensuring complete memory safety and absence of leaks.

### 3. Dijkstra Routing Logistics (`dijkstra-routing`)
*   **Concepts:** Graph Algorithms, Shortest Path, Priority Queues.
*   **Description:** An advanced pathfinding solver using a modified Dijkstra's algorithm. It efficiently calculates optimal delivery routes across a graph of interconnected depots and cities, evaluating paths based on maximum throughput capabilities.

### 4. Templated 2D Sparse Matrix (`templated-matrix`)
*   **Concepts:** Templates, Proxy Classes, Raw Array Manipulation, Operator Overloading.
*   **Description:** A generic, memory-efficient 2D matrix class built around a 1D dynamically allocated raw array. It utilizes the Proxy Design Pattern to overload the double bracket `[][]` operator, ensuring safe and intuitive access to elements within user-defined coordinate bounds.

### 5. Two-Heaps Median Tracker (`two-heaps-median`)
*   **Concepts:** Max-Heap, Min-Heap, Binary Search (`std::lower_bound`).
*   **Description:** A high-performance solution for keeping track of the running median in a continuously updating stream of data. By maintaining balanced Min-Heap and Max-Heap structures, the algorithm guarantees $O(\log N)$ insertion times and $O(1)$ median retrieval.

## Technical Stack
*   **Languages:** C, C++
*   **Memory Analysis:** Valgrind, AddressSanitizer
*   **Build System:** CMake

## Build Instructions
This repository uses a central CMake configuration to build all algorithms at once.

1. Generate build files: `cmake -B build`
2. Compile all projects: `cmake --build build`
3. Executables will be located inside the corresponding module folders within the `build/` directory.

Alternatively, each solution can be compiled: `g++ -std=c++20 -Wall -pedantic main.cpp -o sol` 
and executed: `./sol`

## Disclaimer
*These solutions were developed to demonstrate core computer science principles and software engineering practices in algorithm design and manual memory management.*
