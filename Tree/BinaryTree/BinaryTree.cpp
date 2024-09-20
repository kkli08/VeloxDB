//
// Created by Damian Li on 2024-08-26.
//
#include <iostream>
#include "BinaryTree.h"

// Constructor
BinaryTree::BinaryTree() : root(nullptr) {}

// Destructor
BinaryTree::~BinaryTree() {
    destroyTree(root);
}

TreeNode*& BinaryTree::getRoot() {
    return root;
}

// Helper function to destroy the tree
void BinaryTree::destroyTree(const TreeNode* node) {
    if (node != nullptr) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

// Internal insert method (uses KeyValueWrapper objects)
void BinaryTree::insert(TreeNode*& node, KeyValueWrapper kv) {
    if (node == nullptr) {
        // If the node is null, create a new TreeNode with the KeyValueWrapper
        node = new TreeNode(kv);
    } else if (kv < node->keyValue) {
        // If the key is smaller, insert into the left subtree
        insert(node->left, kv);
    } else {
        // If the key is larger, insert into the right subtree
        insert(node->right, kv);
    }
}


// Search for a key in the tree using KeyValueWrapper
bool BinaryTree::search(const KeyValueWrapper& kv) {
    return search(root, kv);
}

bool BinaryTree::search(TreeNode* node, const KeyValueWrapper& kv) {
    if (node == nullptr) {
        return false;  // Not found
    }

    // Compare the key of the node with the key in the KeyValueWrapper
    if (kv < node->keyValue) {
        return search(node->left, kv);  // Search in the left subtree
    } else if (node->keyValue < kv) {
        return search(node->right, kv);  // Search in the right subtree
    } else {
        // If neither key is less than the other, they are equal
        return node->keyValue == kv;  // Return true if they are equal
    }
}



// Inorder traversal
void BinaryTree::inorderTraversal() {
    inorderTraversal(root);
    std::cout << std::endl;
}

void BinaryTree::inorderTraversal(TreeNode* node) {
    if (node != nullptr) {
        inorderTraversal(node->left);
        node->keyValue.printKeyValue();  // Use the KeyValueWrapper's print method
        inorderTraversal(node->right);
    }
}

// Preorder traversal
void BinaryTree::preorderTraversal() {
    preorderTraversal(root);
    std::cout << std::endl;
}

void BinaryTree::preorderTraversal(TreeNode* node) {
    if (node != nullptr) {
        node->keyValue.printKeyValue();  // Use the KeyValueWrapper's print method
        preorderTraversal(node->left);
        preorderTraversal(node->right);
    }
}

// Postorder traversal
void BinaryTree::postorderTraversal() {
    postorderTraversal(root);
    std::cout << std::endl;
}

void BinaryTree::postorderTraversal(TreeNode* node) {
    if (node != nullptr) {
        postorderTraversal(node->left);
        postorderTraversal(node->right);
        node->keyValue.printKeyValue();  // Use the KeyValueWrapper's print method
    }
}

// Scan between two keys using std::set to maintain sorted order
void BinaryTree::Scan(TreeNode* node, const KeyValueWrapper& small_key, const KeyValueWrapper& large_key, std::set<KeyValueWrapper>& res) {
    if (!node) return;

    // Prune left subtree if the current node's key is greater than the small_key
    if (node->keyValue > small_key) {
        Scan(node->left, small_key, large_key, res);
    }

    // If the current node's key is within the range, add it to the result set
    if (!(node->keyValue < small_key) && !(large_key < node->keyValue)) {
        res.insert(node->keyValue);
    }

    // Prune right subtree if the current node's key is less than the large_key
    if (node->keyValue < large_key) {
        Scan(node->right, small_key, large_key, res);
    }
}
