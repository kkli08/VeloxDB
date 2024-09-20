//
// Created by Damian Li on 2024-08-27.
//
#include <gtest/gtest.h>
#include <string>
#include "RedBlackTree.h"

#include "RedBlackTree.h"

// Test inserting into an empty tree
TEST(RedBlackTreeTest, InsertIntoEmptyTree) {
    RedBlackTree tree;

    // Insert a KeyValueWrapper into the empty tree
    KeyValueWrapper kv(10, "value10");
    tree.insert(kv);

    // Check if the key-value pair was inserted
    KeyValueWrapper result = tree.getValue(kv);
    EXPECT_TRUE(result == kv);
}

// Test inserting with different key types
TEST(RedBlackTreeTest, InsertWithDifferentKeyTypes) {
    RedBlackTree tree;

    // Insert KeyValueWrapper pairs with different key types
    KeyValueWrapper kv1(10, "value10");
    KeyValueWrapper kv2(5.5, "valueDouble");

    tree.insert(kv1);
    tree.insert(kv2);

    // Check if both key-value pairs were inserted
    KeyValueWrapper result1 = tree.getValue(kv1);
    KeyValueWrapper result2 = tree.getValue(kv2);

    EXPECT_TRUE(result1 == kv1);
    EXPECT_TRUE(result2 == kv2);
}

// Test updating an existing key-value pair
TEST(RedBlackTreeTest, UpdateExistedKeyValue) {
    RedBlackTree tree;

    // Insert a KeyValueWrapper pair
    KeyValueWrapper kv1(20, "initialValue");
    tree.insert(kv1);

    // Update the existing key with a new value
    KeyValueWrapper updatedKv(20, "updatedValue");
    tree.insert(updatedKv);  // Should update the value for key 20

    // Check if the value was updated
    KeyValueWrapper result = tree.getValue(updatedKv);
    EXPECT_EQ(result.kv.string_value(), updatedKv.kv.string_value());
}

// Test maintaining red-black tree properties after insertion
TEST(RedBlackTreeTest, InsertMaintainRBTreeProperties) {
    RedBlackTree tree;

    // Insert several KeyValueWrapper pairs
    tree.insert(KeyValueWrapper(15, "value15"));
    tree.insert(KeyValueWrapper(10, "value10"));
    tree.insert(KeyValueWrapper(20, "value20"));

    // The root node should be black to maintain red-black tree properties
    EXPECT_EQ(tree.getColor(tree.getRoot()), BLACK);
}


// Test inserting mixed key types and inorder traversal
TEST(RedBlackTreeTest, InsertMixedKeyTypesAndInorderTraversal) {
    RedBlackTree* tree = new RedBlackTree();

    // Insert different types of KeyValueWrapper pairs
    tree->insert(KeyValueWrapper(10, "value10"));              // int
    tree->insert(KeyValueWrapper(5.5, "valueDouble5.5"));      // double
    tree->insert(KeyValueWrapper("keyString", "valueString")); // string
    tree->insert(KeyValueWrapper('A', "valueCharA"));          // char

    // Capture the inorder traversal output
    std::vector<KeyValueWrapper> inorderResults = tree->inOrderFlushToSst();

    // Check the size of the result
    EXPECT_EQ(inorderResults.size(), 4);

    // Iterate over the results and check the type of each key
    auto it = inorderResults.begin();

    if (it->kv.key_case() == KeyValue::kIntKey) {
        EXPECT_EQ(it->kv.int_key(), 10);
    } else if (it->kv.key_case() == KeyValue::kDoubleKey) {
        EXPECT_EQ(it->kv.double_key(), 5.5);
    } else if (it->kv.key_case() == KeyValue::kStringKey) {
        EXPECT_EQ(it->kv.string_key(), "keyString");
    } else if (it->kv.key_case() == KeyValue::kCharKey) {
        EXPECT_EQ(it->kv.char_key(), std::string(1, 'A'));  // Compare both as std::string
    }

    // Clean up memory
    delete tree;
}


// Test inserting and deleting nodes while checking balancing properties
TEST(RedBlackTreeTest, InsertDeleteNodeAndCheckBalancing) {
    RedBlackTree tree;

    // Insert KeyValueWrapper pairs
    tree.insert(KeyValueWrapper(20, "value20"));
    tree.insert(KeyValueWrapper(10, "value10"));
    tree.insert(KeyValueWrapper(30, "value30"));
    tree.insert(KeyValueWrapper(5, "value5"));
    tree.insert(KeyValueWrapper(15, "value15"));

    // Delete a key from the tree
    tree.deleteKey(KeyValueWrapper(10, ""));

    // Ensure the key is no longer found
    EXPECT_FALSE(tree.search(KeyValueWrapper(10, "")));

    // Ensure other keys are still in the tree
    EXPECT_TRUE(tree.search(KeyValueWrapper(5, "")));
    EXPECT_TRUE(tree.search(KeyValueWrapper(15, "")));
    EXPECT_TRUE(tree.search(KeyValueWrapper(20, "")));
    EXPECT_TRUE(tree.search(KeyValueWrapper(30, "")));

    // Ensure the tree is still balanced and maintains red-black tree properties
    EXPECT_EQ(tree.getColor(tree.getRoot()), BLACK);
    int blackHeight = tree.getBlackHeight(tree.getRoot());
    EXPECT_EQ(tree.getBlackHeight(tree.getRoot()->left) + 1, blackHeight);
    EXPECT_EQ(tree.getBlackHeight(tree.getRoot()->right) + 1, blackHeight);
}


// Test inserting duplicate keys and updating values
TEST(RedBlackTreeTest, InsertDuplicateKeyAndUpdateValue) {
    RedBlackTree tree;

    // Insert a KeyValueWrapper pair
    KeyValueWrapper kv1(42, "initialValue");
    tree.insert(kv1);

    // Insert a KeyValueWrapper pair with the same key but a new value
    KeyValueWrapper kvUpdated(42, "updatedValue");
    tree.insert(kvUpdated);

    // Ensure the value is updated
    KeyValueWrapper result = tree.getValue(kvUpdated);
    EXPECT_EQ(result.kv.string_value(), "updatedValue");

    // Ensure the key is still present
    EXPECT_TRUE(tree.search(KeyValueWrapper(42, "")));
}


// Test a large tree and check black height consistency
TEST(RedBlackTreeTest, LargeTreeAndCheckBlackHeight) {
    RedBlackTree* tree = new RedBlackTree();

    // Insert a large number of nodes
    const int largeTreeSize = 1000;
    for (int i = 1; i <= largeTreeSize; ++i) {
        tree->insert(KeyValueWrapper(i, "value" + std::to_string(i)));
    }

    // Check if the tree is balanced by comparing black heights
    int blackHeight = tree->getBlackHeight(tree->getRoot());
    EXPECT_GT(blackHeight, 0);  // Ensure there's at least one black node

    // Verify that the black height is consistent for both left and right subtrees
    int leftBlackHeight = tree->getBlackHeight(tree->getRoot()->left);
    int rightBlackHeight = tree->getBlackHeight(tree->getRoot()->right);

    EXPECT_EQ(leftBlackHeight, rightBlackHeight);

    // Clean up memory
    delete tree;
}
