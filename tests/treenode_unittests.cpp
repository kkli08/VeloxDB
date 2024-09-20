//
// Created by damian on 9/12/24.
//
#include <gtest/gtest.h>
#include "TreeNode.h"

// Test for TreeNode construction with KeyValueWrapper
TEST(TreeNodeTest, ConstructorWithKeyValueWrapper) {
    KeyValueWrapper kv(42, "Hello");
    TreeNode node(kv);

    EXPECT_EQ(node.keyValue.kv.int_key(), 42);
    EXPECT_EQ(node.keyValue.kv.string_value(), "Hello");
    EXPECT_EQ(node.color, RED);  // Default color is RED
    EXPECT_EQ(node.left, nullptr);
    EXPECT_EQ(node.right, nullptr);
    EXPECT_EQ(node.parent, nullptr);
}

// Test for TreeNode construction with key-value pair (using template)
TEST(TreeNodeTest, ConstructorWithKeyValuePair) {
    TreeNode node(100, "TestValue");

    EXPECT_EQ(node.keyValue.kv.int_key(), 100);
    EXPECT_EQ(node.keyValue.kv.string_value(), "TestValue");
    EXPECT_EQ(node.color, RED);  // Default color is RED
    EXPECT_EQ(node.left, nullptr);
    EXPECT_EQ(node.right, nullptr);
    EXPECT_EQ(node.parent, nullptr);
}

// Test for TreeNode construction with key-value pair and custom color
TEST(TreeNodeTest, ConstructorWithKeyValuePairAndColor) {
    TreeNode node(200, 3.14, BLACK);

    EXPECT_EQ(node.keyValue.kv.int_key(), 200);
    EXPECT_EQ(node.keyValue.kv.double_value(), 3.14);
    EXPECT_EQ(node.color, BLACK);  // Custom color is BLACK
    EXPECT_EQ(node.left, nullptr);
    EXPECT_EQ(node.right, nullptr);
    EXPECT_EQ(node.parent, nullptr);
}

// Test for TreeNode construction with KeyValueWrapper and custom color
TEST(TreeNodeTest, ConstructorWithKeyValueWrapperAndColor) {
    KeyValueWrapper kv("Key", 1234);
    TreeNode node(kv, BLACK);

    EXPECT_EQ(node.keyValue.kv.string_key(), "Key");
    EXPECT_EQ(node.keyValue.kv.int_value(), 1234);
    EXPECT_EQ(node.color, BLACK);  // Custom color is BLACK
    EXPECT_EQ(node.left, nullptr);
    EXPECT_EQ(node.right, nullptr);
    EXPECT_EQ(node.parent, nullptr);
}

// Test for TreeNode pointer relationships
TEST(TreeNodeTest, NodePointerRelationships) {
    KeyValueWrapper kv1(42, "Hello");
    KeyValueWrapper kv2(24, "World");

    TreeNode parent(kv1);
    TreeNode child(kv2);

    parent.left = &child;
    child.parent = &parent;

    EXPECT_EQ(parent.left, &child);
    EXPECT_EQ(child.parent, &parent);
}
