//
// Created by Damian Li on 2024-08-26.
//
#include <gtest/gtest.h>
#include "BinaryTree.h"

#include <gtest/gtest.h>
#include "BinaryTree.h"

// Test inserting and searching an integer key
TEST(BinaryTreeTest, InsertAndSearchInt) {
    BinaryTree tree;
    tree.insert(42, "TestValue");

    // Test if the inserted key can be found
    EXPECT_TRUE(tree.search(42));

    // Test if a non-existent key is not found
    EXPECT_FALSE(tree.search(100));
}

// Test inserting and searching a double key
TEST(BinaryTreeTest, InsertAndSearchDouble) {
    BinaryTree tree;
    tree.insert(3.14, "Pi");

    // Test if the inserted key can be found
    EXPECT_TRUE(tree.search(3.14));

    // Test if a non-existent key is not found
    EXPECT_FALSE(tree.search(2.71));
}

// Test inserting and searching a string key
TEST(BinaryTreeTest, InsertAndSearchString) {
    BinaryTree tree;
    tree.insert("Key", 123);

    // Test if the inserted key can be found
    EXPECT_TRUE(tree.search("Key"));

    // Test if a non-existent key is not found
    EXPECT_FALSE(tree.search("NonExistent"));
}

// Test inserting and searching a char key
TEST(BinaryTreeTest, InsertAndSearchChar) {
    BinaryTree tree;
    tree.insert('A', 100);
    tree.insert(10, 101);

    // Test if the inserted key can be found
    EXPECT_TRUE(tree.search('A'));

    // Test if a non-existent key is not found
    EXPECT_FALSE(tree.search('C'));
}

// Test inserting multiple keys of different types
TEST(BinaryTreeTest, InsertMultipleTypes) {
    BinaryTree tree;
    tree.insert(42, "IntValue");
    tree.insert(3.14, "DoubleValue");
    tree.insert("Key", 123);
    tree.insert('A', "CharValue");

    // Test if all inserted keys can be found
    EXPECT_TRUE(tree.search(42));
    EXPECT_TRUE(tree.search(3.14));
    EXPECT_TRUE(tree.search("Key"));
    EXPECT_TRUE(tree.search('A'));

    // Test if non-existent keys are not found
    EXPECT_FALSE(tree.search(100));
    EXPECT_FALSE(tree.search("NonExistent"));
}

// Test searching in an empty tree
TEST(BinaryTreeTest, SearchInEmptyTree) {
    BinaryTree tree;

    // Test if searching in an empty tree returns false
    EXPECT_FALSE(tree.search(42));
    EXPECT_FALSE(tree.search("Key"));
    EXPECT_FALSE(tree.search(3.14));
}

// Test inserting and searching a long long int key
TEST(BinaryTreeTest, InsertAndSearchLongLong) {
    BinaryTree tree;
    tree.insert(9223372036854775807LL, "LongLongMax");

    // Test if the inserted key can be found
    EXPECT_TRUE(tree.search(9223372036854775807LL));

    // Test if a non-existent key is not found
    EXPECT_FALSE(tree.search(9223372036854775806LL));
}

// Test inserting and searching with duplicate keys
TEST(BinaryTreeTest, InsertAndSearchDuplicateKeys) {
    BinaryTree tree;
    tree.insert(42, "FirstValue");
    tree.insert(42, "SecondValue");  // Duplicate key

    // Test if the key can be found after inserting a duplicate
    EXPECT_TRUE(tree.search(42));

    // Since the tree structure doesn't specify handling duplicates,
    // the test checks that the key exists (specific value handling depends on the implementation).
}


/*
 * Scan
 */
// Test case 1: Scan for keys within the range
TEST(BinaryTreeTest, ScanWithinRange) {
    BinaryTree tree;
    tree.insert(10, "Value10");
    tree.insert(5, "Value5");
    tree.insert(15, "Value15");
    tree.insert(3, "Value3");
    tree.insert(7, "Value7");

    KeyValueWrapper small_key(4, 0);
    KeyValueWrapper large_key(12, 0);
    std::set<KeyValueWrapper> result;

    // Perform the scan
    tree.Scan(tree.getRoot(), small_key, large_key, result);

    // Expected result: 5, 7, 10
    EXPECT_EQ(result.size(), 3);
    EXPECT_TRUE(result.count(KeyValueWrapper(5, "Value5")));
    EXPECT_TRUE(result.count(KeyValueWrapper(7, "Value7")));
    EXPECT_TRUE(result.count(KeyValueWrapper(10, "Value10")));
}

// Test case 2: Scan with no keys in the range
TEST(BinaryTreeTest, ScanWithNoKeysInRange) {
    BinaryTree tree;
    tree.insert(10, "Value10");
    tree.insert(5, "Value5");
    tree.insert(15, "Value15");

    KeyValueWrapper small_key(20, 0);  // Range starts above any key in the tree
    KeyValueWrapper large_key(30, 0);
    std::set<KeyValueWrapper> result;

    // Perform the scan
    tree.Scan(tree.getRoot(), small_key, large_key, result);

    // Expected result: no keys in this range
    EXPECT_EQ(result.size(), 0);
}

// Test case 3: Scan with all keys within the range
TEST(BinaryTreeTest, ScanAllKeysInRange) {
    BinaryTree tree;
    tree.insert(10, "Value10");
    tree.insert(5, "Value5");
    tree.insert(15, "Value15");

    KeyValueWrapper small_key(0, 0);  // Range covers all keys
    KeyValueWrapper large_key(20, 0);
    std::set<KeyValueWrapper> result;

    // Perform the scan
    tree.Scan(tree.getRoot(), small_key, large_key, result);

    // Expected result: all keys (5, 10, 15)
    EXPECT_EQ(result.size(), 3);
    EXPECT_TRUE(result.count(KeyValueWrapper(5, "Value5")));
    EXPECT_TRUE(result.count(KeyValueWrapper(10, "Value10")));
    EXPECT_TRUE(result.count(KeyValueWrapper(15, "Value15")));
}

// Test case 4: Scan with edge keys at the boundary of the range
TEST(BinaryTreeTest, ScanWithEdgeKeys) {
    BinaryTree tree;
    tree.insert(10, "Value10");
    tree.insert(5, "Value5");
    tree.insert(15, "Value15");

    KeyValueWrapper small_key(5, 0);  // Exact match at the lower bound
    KeyValueWrapper large_key(15, 0);  // Exact match at the upper bound
    std::set<KeyValueWrapper> result;

    // Perform the scan
    tree.Scan(tree.getRoot(), small_key, large_key, result);

    // Expected result: keys (5, 10, 15)
    EXPECT_EQ(result.size(), 3);
    EXPECT_TRUE(result.count(KeyValueWrapper(5, "Value5")));
    EXPECT_TRUE(result.count(KeyValueWrapper(10, "Value10")));
    EXPECT_TRUE(result.count(KeyValueWrapper(15, "Value15")));
}
