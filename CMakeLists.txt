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

# Include FetchContent module
include(FetchContent)

# Fetch Abseil library
FetchContent_Declare(
        abseil
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG        20230802.0  # Use the latest stable release tag
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        GIT_CLONE_TIMEOUT 300      # Increase timeout as needed
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(abseil)

# Fetch GoogleTest
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.1
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
        GIT_CLONE_TIMEOUT 300      # Increase timeout as needed
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Specify the path to the .proto files
set(PROTO_FILES
        ${PROJECT_SOURCE_DIR}/protobuf/KeyValue.proto
)

# Generate .pb.cc and .pb.h files from .proto files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Ensure that the generated files are placed in the protobuf directory
set(GENERATED_PROTO_DIR ${PROJECT_SOURCE_DIR}/protobuf)

# ---- DEFINE SHARED LIBRARY ----
add_library(veloxdb_lib STATIC
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
        Storage/BloomFilter/BloomFilter.cpp

        # VeloxDB
        VeloxDB/VeloxDB.cpp
        VeloxDB/VeloxDB.h
        VeloxDB/VeloxDB.tpp

        # LSMTree
        LSMTree/LSMTree.cpp
        LSMTree/LSMTree.h

        # Generated Protobuf source files
        ${PROTO_SRCS}
)

# Include directories for veloxdb_lib
target_include_directories(veloxdb_lib PUBLIC
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
        ${PROJECT_SOURCE_DIR}/LSMTree
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
        ${GENERATED_PROTO_DIR}
        ${Protobuf_INCLUDE_DIRS}
        ${absl_SOURCE_DIR}
        ${absl_BINARY_DIR}
)

# Link libraries to veloxdb_lib
target_link_libraries(veloxdb_lib PUBLIC
        protobuf::libprotobuf
        absl::check
        absl::log
        absl::strings
        absl::status
        absl::synchronization
        absl::time
        absl::memory
        absl::flat_hash_map
        absl::base
)

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
        tests/bloom_filter_unittests.cpp
)

# Include directories for runTests
target_include_directories(runTests PRIVATE
        ${PROJECT_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Link libraries to runTests
target_link_libraries(runTests PRIVATE
        veloxdb_lib
        gtest
        gtest_main
)

# Register tests with CTest
add_test(NAME runTests COMMAND runTests)

# ---- MAIN TARGET ----
add_executable(main
        main.cpp
)

# Include directories for main
target_include_directories(main PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Link libraries to main
target_link_libraries(main PRIVATE
        veloxdb_lib
)

# Include the Benchmark directory
add_subdirectory(Benchmark)
