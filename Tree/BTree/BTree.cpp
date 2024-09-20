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
        root->keys.push_back(kv);
    } else {
        if (root->keys.size() == 2 * degree - 1) {
            // Root is full, need to split
            BTreeNode* newRoot = createNode(false);
            newRoot->children.push_back(root);
            newRoot->splitChild(0, degree);

            // Decide which child will have the new key
            int i = 0;
            if (newRoot->keys[0] < kv)
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
    int i = keys.size() - 1;

    if (isLeaf) {
        // Insert into leaf node
        keys.push_back(kv); // Temporarily add at the end
        while (i >= 0 && kv < keys[i]) {
            keys[i + 1] = keys[i];
            i--;
        }
        keys[i + 1] = kv;
    } else {
        // Find the child that will have the new key
        while (i >= 0 && kv < keys[i])
            i--;

        i++;
        // Check if the found child is full
        if (children[i]->keys.size() == 2 * degree - 1) {
            splitChild(i, degree);
            if (kv > keys[i])
                i++;
        }
        children[i]->insertNonFull(kv, degree);
    }
}

// Split the child node
void BTree::BTreeNode::splitChild(int idx, int degree) {
    BTreeNode* child = children[idx];
    BTreeNode* sibling = new BTreeNode(child->isLeaf);

    sibling->next = child->next;
    child->next = sibling;

    int mid = degree - 1; // Middle index

    // For leaf nodes
    if (child->isLeaf) {
        // Move the second half keys to sibling
        sibling->keys.assign(child->keys.begin() + mid, child->keys.end());
        child->keys.resize(mid);

        // Insert the first key of sibling to parent keys
        keys.insert(keys.begin() + idx, sibling->keys[0]);

        // Insert sibling into children
        children.insert(children.begin() + idx + 1, sibling);
    } else {
        // For internal nodes
        // Move the second half keys and children to sibling
        sibling->keys.assign(child->keys.begin() + degree, child->keys.end());
        sibling->children.assign(child->children.begin() + degree, child->children.end());

        // Reduce the size of child
        child->keys.resize(degree - 1);
        child->children.resize(degree);

        // Move the middle key up to the parent
        keys.insert(keys.begin() + idx, child->keys[mid]);

        // Insert sibling into children
        children.insert(children.begin() + idx + 1, sibling);
    }
}

// Search for a key in the B+ Tree
KeyValueWrapper* BTree::search(const KeyValueWrapper& kv) {
    return (root == nullptr) ? nullptr : root->search(kv);
}

// Search for a key in the subtree rooted with this node
KeyValueWrapper* BTree::BTreeNode::search(const KeyValueWrapper& kv) {
    int i = 0;

    // Find the first key greater than or equal to kv
    while (i < keys.size() && kv > keys[i])
        i++;

    if (isLeaf) {
        if (i < keys.size() && !(kv < keys[i]) && !(keys[i] < kv)) {
            // Found exact key
            return &keys[i];
        }
        // Not found
        return nullptr;
    } else {
        // If key is found in internal node, proceed to child
        return children[i]->search(kv);
    }
}

// Traverse and print the B+ Tree
void BTree::traverse() const {
    if (root != nullptr)
        root->traverse();
}

// Traverse the subtree rooted with this node
void BTree::BTreeNode::traverse() const {
    int i;
    if (isLeaf) {
        // Print all keys in leaf node
        for (i = 0; i < keys.size(); i++) {
            keys[i].printKeyValue();
        }
        if (next != nullptr) {
            // Optionally, print separator between leaves
            std::cout << " -> ";
            next->traverse();
        }
    } else {
        // Traverse all child nodes
        for (i = 0; i < keys.size(); i++) {
            children[i]->traverse();
            // Optionally, print keys of internal nodes
            // std::cout << "Key: ";
            // keys[i].printKeyValue();
        }
        // Traverse the last child
        children[i]->traverse();
    }
}

