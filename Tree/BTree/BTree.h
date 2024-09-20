//
// Created by Damian Li on 2024-09-20.
//

#ifndef BTREE_H
#define BTREE_H

#include "KeyValue.h"
#include <vector>
#include <string>

class BTree {
public:
    // Constructor: Initialize the B+ Tree with a given degree
    BTree(int degree);

    // Insert a key-value pair into the B+ Tree
    void insert(const KeyValueWrapper& kv);

    // Search for a key in the B+ Tree
    KeyValueWrapper* search(const std::string& key);

    // Traverse and print the B+ Tree
    void traverse() const;

private:
    // B+ Tree node structure
    struct BTreeNode {
        bool isLeaf;                           // True if node is a leaf
        std::vector<std::string> keys;         // Keys for internal nodes
        std::vector<KeyValueWrapper> keyValues; // Key-value pairs for leaf nodes
        std::vector<BTreeNode*> children;      // Child pointers
        BTreeNode* next;                       // Pointer to next leaf node

        // Constructor
        BTreeNode(bool leaf);

        // Insert a key-value pair when node is not full
        void insertNonFull(const KeyValueWrapper& kv, int degree);

        // Split the child node
        void splitChild(int idx, int degree);

        // Search for a key in the subtree rooted with this node
        KeyValueWrapper* search(const std::string& key);

        // Traverse the subtree rooted with this node
        void traverse() const;
    };

    BTreeNode* root; // Pointer to the root node
    int degree;      // Minimum degree

    // Helper function to create a new node
    BTreeNode* createNode(bool isLeaf);
};

#endif // BTREE_H
