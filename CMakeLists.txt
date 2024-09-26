cmake_minimum_required(VERSION 3.10)

# Project name
project(VeloxDB)

# Set the required C++ standard and features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the required protobuf package
find_package(Protobuf REQUIRED)

# Specify the path to the .proto files
set(PROTO_FILES
        ${PROJECT_SOURCE_DIR}/protobuf/KeyValue.proto
)

# Set the output directory for the generated .pb.cc and .pb.h files
set(GENERATED_PROTO_DIR ${PROJECT_SOURCE_DIR}/protobuf)

# Command to generate .pb.cc and .pb.h files from .proto files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# Ensure that the generated files are placed in the protobuf directory
set(PROTO_SRCS
        ${GENERATED_PROTO_DIR}/KeyValue.pb.cc
)
set(PROTO_HDRS
        ${GENERATED_PROTO_DIR}/KeyValue.pb.h
)

# Include GoogleTest via FetchContent
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
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


        # Source files required by tests
        # kv
        kv/KeyValue.cpp
        kv/KeyValue.tpp
        # Memory
        Memory/Memtable/Memtable.cpp
        # Tree
        Tree/TreeNode/TreeNode.cpp
        Tree/TreeNode/TreeNode.tpp
        Tree/BinaryTree/BinaryTree.cpp
        Tree/BinaryTree/BinaryTree.tpp
        Tree/RedBlackTree/RedBlackTree.cpp
        Tree/BTree/BTree.cpp
        # Storage
        Storage/FileManager/FileManager.cpp

        ${PROTO_SRCS}
        Storage/Page/Page.cpp
        Storage/Page/Page.h
        Storage/PageManager/PageManager.cpp
        Storage/PageManager/PageManager.h
        Storage/DiskBTree/DiskBTree.cpp
        Storage/DiskBTree/DiskBTree.h
        Storage/SstFileManager/SstFileManager.cpp
        Storage/SstFileManager/SstFileManager.h
        tests/page_unittests.cpp
        tests/page_manager_unittest.cpp
        tests/DiskBTree_unittest.cpp
        tests/SST_File_Manager_unittest.cpp
        VeloxDB/VeloxDB.h
        VeloxDB/VeloxDB.cpp
        VeloxDB/VeloxDB.tpp
        tests/VeloxDB_api_unittest.cpp
        Memory/BufferPool/BufferPool.cpp

)

# Link Google Test and OpenSSL to the test executable
target_link_libraries(runTests gtest gtest_main ${Protobuf_LIBRARIES})

# Register tests with CTest
add_test(NAME runTests COMMAND runTests)

# ---- MAIN TARGET ----
# List of source files
set(SOURCE_FILES
        main.cpp
        Memory/Memtable/Memtable.cpp

        kv/KeyValue.cpp

        Tree/BTree/BTree.cpp
        Tree/BTree/BTree.h
        Tree/RedBlackTree/RedBlackTree.cpp
        Tree/TreeNode/TreeNode.cpp
        Tree/BinaryTree/BinaryTree.cpp
        Storage/Page/Page.cpp
        Storage/Page/Page.h
        Storage/PageManager/PageManager.cpp
        Storage/PageManager/PageManager.h
        Storage/DiskBTree/DiskBTree.cpp
        Storage/DiskBTree/DiskBTree.h
        Storage/SstFileManager/SstFileManager.cpp
        Storage/SstFileManager/SstFileManager.h
        VeloxDB/VeloxDB.h
        VeloxDB/VeloxDB.cpp
        VeloxDB/VeloxDB.tpp
        Memory/BufferPool/BufferPool.cpp
)

# Add the executable
add_executable(main ${SOURCE_FILES} ${PROTO_SRCS})

# Link Protobuf and OpenSSL to the main target
target_link_libraries(main ${Protobuf_LIBRARIES})

# Include directories (header files)
include_directories(
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
)

# Add the generated protobuf directory to the include path
include_directories(${GENERATED_PROTO_DIR})

