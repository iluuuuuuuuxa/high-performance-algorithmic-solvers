cmake_minimum_required(VERSION 3.20)
project(HighPerformanceAlgorithmicSolvers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_subdirectory(avl-interval-tree)
add_subdirectory(custom-hash-table)
add_subdirectory(dijkstra-routing)
add_subdirectory(templated-2d-matrix)
add_subdirectory(two-heaps-median)