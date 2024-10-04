# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(Benchmark)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Protobuf package
find_package(Protobuf REQUIRED)

# Specify the path to the .proto files
set(PROTO_FILES
        ${PROJECT_SOURCE_DIR}/../protobuf/KeyValue.proto
)

# Set the output directory for the generated .pb.cc and .pb.h files
set(GENERATED_PROTO_DIR ${PROJECT_SOURCE_DIR}/../protobuf)

# Include the generated protobuf files
set(PROTO_SRCS
        ${GENERATED_PROTO_DIR}/KeyValue.pb.cc
)
set(PROTO_HDRS
        ${GENERATED_PROTO_DIR}/KeyValue.pb.h
)

# Include directories
include_directories(
        ${PROJECT_SOURCE_DIR}/../protobuf
        ${PROJECT_SOURCE_DIR}/../kv
        ${PROJECT_SOURCE_DIR}/../Memory/Memtable
        ${PROJECT_SOURCE_DIR}/../Memory/BufferPool
        ${PROJECT_SOURCE_DIR}/../Storage/Page
        ${PROJECT_SOURCE_DIR}/../Storage/PageManager
        ${PROJECT_SOURCE_DIR}/../Storage/SstFileManager
        ${PROJECT_SOURCE_DIR}/../Storage/FileManager
        ${PROJECT_SOURCE_DIR}/../Storage/DiskBTree
        ${PROJECT_SOURCE_DIR}/../Tree/BinaryTree
        ${PROJECT_SOURCE_DIR}/../Tree/BTree
        ${PROJECT_SOURCE_DIR}/../Tree/RedBlackTree
        ${PROJECT_SOURCE_DIR}/../Tree/TreeNode
        ${PROJECT_SOURCE_DIR}/../VeloxDB
        ${PROJECT_SOURCE_DIR}
        ${GENERATED_PROTO_DIR}  # Include generated protobuf directory
)

# List of source files for VeloxDB
set(VELOXDB_SRCS
        put_throughput_benchmark.cpp
        ../Memory/Memtable/Memtable.cpp
        ../kv/KeyValue.cpp
        ../Tree/BTree/BTree.cpp
        ../Tree/RedBlackTree/RedBlackTree.cpp
        ../Tree/TreeNode/TreeNode.cpp
        ../Tree/BinaryTree/BinaryTree.cpp
        ../Storage/Page/Page.cpp
        ../Storage/PageManager/PageManager.cpp
        ../Storage/DiskBTree/DiskBTree.cpp
        ../Storage/SstFileManager/SstFileManager.cpp
        ../VeloxDB/VeloxDB.cpp
        ../Memory/BufferPool/BufferPool.cpp
)

# Specify the source files for the benchmark
set(BENCHMARK_SRCS
        put_throughput_benchmark.cpp  # Ensure this file contains `main` function
)

# Add executable for the benchmark, ensuring main function is included
add_executable(put_benchmark ${BENCHMARK_SRCS} ${PROTO_SRCS} ${VELOXDB_SRCS})

# Link libraries for the benchmark executable
target_link_libraries(put_benchmark ${Protobuf_LIBRARIES})

