//
// Created by Damian Li on 2024-09-20.
//

#include "BTree.h"
#include <iostream>

// Constructor
BTree::BTree(int degree) : root(nullptr), degree(degree) {}

// Create a new node
BTree::BTreeNode* BTree::createNode(bool isLeaf) {
    BTreeNode* node = new BTreeNode(isLeaf);
    return node;
}

// BTreeNode constructor
BTree::BTreeNode::BTreeNode(bool leaf) : isLeaf(leaf), next(nullptr) {}

// Insert a key-value pair into the B+ Tree
void BTree::insert(const KeyValueWrapper& kv) {
    if (root == nullptr) {
        // Tree is empty, allocate root
        root = createNode(true);
        root->keyValues.push_back(kv);
    } else {
        if (root->isLeaf && root->keyValues.size() == 2 * degree - 1) {
            // Root is full, need to split
            BTreeNode* newRoot = createNode(false);
            newRoot->children.push_back(root);
            newRoot->splitChild(0, degree);
            int i = 0;
            if (newRoot->keys[0] < kv.kv.key())
                i++;
            newRoot->children[i]->insertNonFull(kv, degree);
            root = newRoot;
        } else {
            root->insertNonFull(kv, degree);
        }
    }
}

// Insert a key-value pair when node is not full
void BTree::BTreeNode::insertNonFull(const KeyValueWrapper& kv, int degree) {
    if (isLeaf) {
        // Insert into leaf node
        auto it = keyValues.begin();
        while (it != keyValues.end() && it->kv.key() < kv.kv.key())
            ++it;
        keyValues.insert(it, kv);
    } else {
        // Insert into internal node
        int idx = 0;
        while (idx < keys.size() && kv.kv.key() > keys[idx])
            idx++;
        if (children[idx]->isLeaf && children[idx]->keyValues.size() == 2 * degree - 1) {
            splitChild(idx, degree);
            if (kv.kv.key() > keys[idx])
                idx++;
        }
        children[idx]->insertNonFull(kv, degree);
    }
}

// Split the child node
void BTree::BTreeNode::splitChild(int idx, int degree) {
    BTreeNode* child = children[idx];
    BTreeNode* sibling = new BTreeNode(child->isLeaf);
    sibling->next = child->next;
    child->next = sibling;

    if (child->isLeaf) {
        // Split leaf node
        sibling->keyValues.assign(child->keyValues.begin() + degree, child->keyValues.end());
        child->keyValues.resize(degree);
        keys.insert(keys.begin() + idx, sibling->keyValues[0].kv.key());
        children.insert(children.begin() + idx + 1, sibling);
    } else {
        // Split internal node
        sibling->keys.assign(child->keys.begin() + degree, child->keys.end());
        sibling->children.assign(child->children.begin() + degree, child->children.end());
        child->keys.resize(degree - 1);
        child->children.resize(degree);
        keys.insert(keys.begin() + idx, child->keys[degree - 1]);
        children.insert(children.begin() + idx + 1, sibling);
    }
}

// Search for a key in the B+ Tree
KeyValueWrapper* BTree::search(const std::string& key) {
    return (root == nullptr) ? nullptr : root->search(key);
}

// Search for a key in the subtree rooted with this node
KeyValueWrapper* BTree::BTreeNode::search(const std::string& key) {
    if (isLeaf) {
        for (auto& kv : keyValues) {
            if (kv.kv.key() == key)
                return &kv;
        }
        return nullptr;
    } else {
        int idx = 0;
        while (idx < keys.size() && key > keys[idx])
            idx++;
        return children[idx]->search(key);
    }
}

// Traverse and print the B+ Tree
void BTree::traverse() const {
    if (root != nullptr)
        root->traverse();
}

// Traverse the subtree rooted with this node
void BTree::BTreeNode::traverse() const {
    if (isLeaf) {
        for (const auto& kv : keyValues)
            kv.printKeyValue();
        if (next != nullptr) {
            // Optionally, print separator between leaves
            std::cout << " -> ";
            next->traverse();
        }
    } else {
        for (int i = 0; i < keys.size(); ++i) {
            children[i]->traverse();
            // Optionally, print keys of internal nodes
            // std::cout << "Key: " << keys[i] << std::endl;
        }
        children[keys.size()]->traverse();
    }
}
