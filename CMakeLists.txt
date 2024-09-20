cmake_minimum_required(VERSION 3.10)

# Project name
project(VeloxDB)

# Set the required C++ standard and features
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Find the required protobuf package
#find_package(Protobuf REQUIRED)
#
## Specify the path to the .proto files
#set(PROTO_FILES
#        ${PROJECT_SOURCE_DIR}/protobuf/KeyValue.proto
#        ${PROJECT_SOURCE_DIR}/protobuf/SSTIndex.proto
#)
#
## Set the output directory for the generated .pb.cc and .pb.h files
#set(GENERATED_PROTO_DIR ${PROJECT_SOURCE_DIR}/protobuf)
#
## Command to generate .pb.cc and .pb.h files from .proto files
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

## Ensure that the generated files are placed in the protobuf directory
#set(PROTO_SRCS
#        ${GENERATED_PROTO_DIR}/KeyValue.pb.cc
#        ${GENERATED_PROTO_DIR}/SSTIndex.pb.cc
#)
#set(PROTO_HDRS
#        ${GENERATED_PROTO_DIR}/KeyValue.pb.h
#        ${GENERATED_PROTO_DIR}/SSTIndex.pb.h
#)

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
#add_executable(runTests
#        # Test files
#        tests/binarytree_tests.cpp
#        tests/redblacktree_unittest.cpp
#        tests/api_unittest.cpp
#        tests/memtable_unittest.cpp
#        tests/sstIndex_unittest.cpp
#        tests/kvpair_unittest.cpp
#        tests/file_manager_unittest.cpp
#        tests/bufferpool_unittest.cpp
#
#        # Source files required by tests
#        tree/BinaryTree.cpp
#        tree/TreeNode.cpp
#        tree/RedBlackTree.cpp
#        api/api.cpp
#        memtable/Memtable.cpp
#        SSTIndex/SSTIndex.cpp
#        FileManager/FileManager.cpp
#        kv/KeyValue.cpp
#        ${PROTO_SRCS}
#        tests/treenode_unittests.cpp
#        bufferPool/BufferPool.cpp
#        bufferPool/BufferPool.h
#        tree/BTree.cpp
#        tree/BTree.h
#        tests/bufferpool_unittest.cpp
#)

## Link Google Test and OpenSSL to the test executable
#target_link_libraries(runTests gtest gtest_main ${Protobuf_LIBRARIES})
#
## Register tests with CTest
#add_test(NAME runTests COMMAND runTests)

# ---- MAIN TARGET ----
# List of source files
set(SOURCE_FILES
        main.cpp
#        api/api.cpp
#        memtable/Memtable.cpp
#        tree/BinaryTree.cpp
#        tree/TreeNode.cpp
#        tree/RedBlackTree.cpp
#        SSTIndex/SSTIndex.cpp
#        kv/KeyValue.cpp
#        FileManager/FileManager.cpp
#        bufferPool/BufferPool.cpp
#        bufferPool/BufferPool.h
#        tree/BTree.cpp
#        tree/BTree.h
)

# Add the executable
add_executable(main ${SOURCE_FILES} ${PROTO_SRCS})

# Link Protobuf and OpenSSL to the main target
target_link_libraries(main ${Protobuf_LIBRARIES})

# Include directories (header files)
include_directories(
#        ${PROJECT_SOURCE_DIR}/api
#        ${PROJECT_SOURCE_DIR}/memtable
#        ${PROJECT_SOURCE_DIR}/tree
#        ${PROJECT_SOURCE_DIR}/SSTIndex
#        ${PROJECT_SOURCE_DIR}/kv
#        ${PROJECT_SOURCE_DIR}/FileManager
#        ${PROJECT_SOURCE_DIR}/protobuf
#        ${PROJECT_SOURCE_DIR}/bufferPool
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
)

## Add the generated protobuf directory to the include path
#include_directories(${GENERATED_PROTO_DIR})

