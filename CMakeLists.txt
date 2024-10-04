cmake_minimum_required(VERSION 3.14)

# Project name
project(VeloxDB)

# Set the required C++ standard and features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enforce the use of arm64 architecture on macOS
if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES arm64)
endif()

# Find the required Protobuf package
find_package(Protobuf REQUIRED)

# Specify the path to the .proto files
set(PROTO_FILES
        ${PROJECT_SOURCE_DIR}/protobuf/KeyValue.proto
)

# Generate .pb.cc and .pb.h files from .proto files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Ensure that the generated files are placed in the protobuf directory
set(GENERATED_PROTO_DIR ${PROJECT_SOURCE_DIR}/protobuf)

# Include FetchContent module
include(FetchContent)

# Fetch Abseil library
FetchContent_Declare(
        abseil
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG        20230802.0  # Use the latest stable release tag
)
FetchContent_MakeAvailable(abseil)

# Fetch GoogleTest
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# ---- TEST TARGET ----
add_executable(runTests
        # Test files
        tests/binarytree_tests.cpp
        tests/redblacktree_unittest.cpp
        tests/memtable_unittest.cpp
        tests/kvpair_unittest.cpp
        tests/treenode_unittests.cpp
        tests/page_unittests.cpp
        tests/page_manager_unittest.cpp
        tests/DiskBTree_unittest.cpp
        tests/SST_File_Manager_unittest.cpp
        tests/VeloxDB_api_unittest.cpp
        tests/veloxdb_GET_benchmark.cpp

        # Source files required by tests
        # kv
        kv/KeyValue.cpp
        kv/KeyValue.tpp

        # Memory
        Memory/Memtable/Memtable.cpp
        Memory/BufferPool/BufferPool.cpp

        # Tree
        Tree/TreeNode/TreeNode.cpp
        Tree/TreeNode/TreeNode.tpp
        Tree/BinaryTree/BinaryTree.cpp
        Tree/BinaryTree/BinaryTree.tpp
        Tree/RedBlackTree/RedBlackTree.cpp
        Tree/BTree/BTree.cpp

        # Storage
        Storage/FileManager/FileManager.cpp
        Storage/Page/Page.cpp
        Storage/PageManager/PageManager.cpp
        Storage/DiskBTree/DiskBTree.cpp
        Storage/SstFileManager/SstFileManager.cpp

        # VeloxDB
        VeloxDB/VeloxDB.cpp
        VeloxDB/VeloxDB.h
        VeloxDB/VeloxDB.tpp

        # Generated Protobuf source files
        ${PROTO_SRCS}
)

# Include directories for runTests
target_include_directories(runTests PRIVATE
        ${PROJECT_SOURCE_DIR}/kv
        ${PROJECT_SOURCE_DIR}/Memory/Memtable
        ${PROJECT_SOURCE_DIR}/Memory/BufferPool
        ${PROJECT_SOURCE_DIR}/Storage/BloomFilter
        ${PROJECT_SOURCE_DIR}/Storage/Page
        ${PROJECT_SOURCE_DIR}/Storage/PageManager
        ${PROJECT_SOURCE_DIR}/Storage/SstFileManager
        ${PROJECT_SOURCE_DIR}/Storage/FileManager
        ${PROJECT_SOURCE_DIR}/Storage/DiskBTree
        ${PROJECT_SOURCE_DIR}/Tree/BinaryTree
        ${PROJECT_SOURCE_DIR}/Tree/BTree
        ${PROJECT_SOURCE_DIR}/Tree/LSMTree
        ${PROJECT_SOURCE_DIR}/Tree/RedBlackTree
        ${PROJECT_SOURCE_DIR}/Tree/TreeNode
        ${PROJECT_SOURCE_DIR}/VeloxDB
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
        ${GENERATED_PROTO_DIR}
        ${Protobuf_INCLUDE_DIRS}
        ${absl_SOURCE_DIR}
        ${absl_BINARY_DIR}
)

# Link libraries to runTests
target_link_libraries(runTests PRIVATE
        gtest
        gtest_main
        protobuf::libprotobuf
        absl::check    # Corrected from absl::checks to absl::check
        absl::log
        absl::strings
        absl::status
        absl::synchronization
        absl::time
        absl::memory
        absl::flat_hash_map
        absl::base
)

# Register tests with CTest
add_test(NAME runTests COMMAND runTests)

# ---- MAIN TARGET ----
add_executable(main
        main.cpp

        # Source files
        kv/KeyValue.cpp
        kv/KeyValue.tpp

        Memory/Memtable/Memtable.cpp
        Memory/BufferPool/BufferPool.cpp

        Tree/TreeNode/TreeNode.cpp
        Tree/TreeNode/TreeNode.tpp
        Tree/BinaryTree/BinaryTree.cpp
        Tree/BinaryTree/BinaryTree.tpp
        Tree/RedBlackTree/RedBlackTree.cpp
        Tree/BTree/BTree.cpp

        Storage/FileManager/FileManager.cpp
        Storage/Page/Page.cpp
        Storage/PageManager/PageManager.cpp
        Storage/DiskBTree/DiskBTree.cpp
        Storage/SstFileManager/SstFileManager.cpp

        VeloxDB/VeloxDB.cpp
        VeloxDB/VeloxDB.h
        VeloxDB/VeloxDB.tpp

        # Generated Protobuf source files
        ${PROTO_SRCS}
)

# Include directories for main
target_include_directories(main PRIVATE
        ${PROJECT_SOURCE_DIR}/kv
        ${PROJECT_SOURCE_DIR}/Memory/Memtable
        ${PROJECT_SOURCE_DIR}/Memory/BufferPool
        ${PROJECT_SOURCE_DIR}/Storage/BloomFilter
        ${PROJECT_SOURCE_DIR}/Storage/Page
        ${PROJECT_SOURCE_DIR}/Storage/PageManager
        ${PROJECT_SOURCE_DIR}/Storage/SstFileManager
        ${PROJECT_SOURCE_DIR}/Storage/FileManager
        ${PROJECT_SOURCE_DIR}/Storage/DiskBTree
        ${PROJECT_SOURCE_DIR}/Tree/BinaryTree
        ${PROJECT_SOURCE_DIR}/Tree/BTree
        ${PROJECT_SOURCE_DIR}/Tree/LSMTree
        ${PROJECT_SOURCE_DIR}/Tree/RedBlackTree
        ${PROJECT_SOURCE_DIR}/Tree/TreeNode
        ${PROJECT_SOURCE_DIR}/VeloxDB
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
        ${GENERATED_PROTO_DIR}
        ${Protobuf_INCLUDE_DIRS}
        ${absl_SOURCE_DIR}
        ${absl_BINARY_DIR}
)

# Link libraries to main
target_link_libraries(main PRIVATE
        protobuf::libprotobuf
        absl::check    # Corrected from absl::checks to absl::check
        absl::log
        absl::strings
        absl::status
        absl::synchronization
        absl::time
        absl::memory
        absl::flat_hash_map
        absl::base
)

# Optionally include the Benchmark directory
# Uncomment the following line if you have a Benchmark directory to include
# add_subdirectory(Benchmark)
