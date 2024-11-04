//
// Created by Damian Li on 2024-08-26.
//

// Memtable.cpp
#include "Memtable.h"

// Default Constructor
Memtable::Memtable()
    : memtableSize(1000), // Default threshold
      currentSize(0),
      tree(new RedBlackTree()) {
}

// Constructor with threshold
Memtable::Memtable(int threshold)
    : memtableSize(threshold),
      currentSize(0),
      tree(new RedBlackTree()) {
}

// Destructor
Memtable::~Memtable() {
    delete tree;
}

// Insert a key-value pair into the memtable
void Memtable::put(const KeyValueWrapper& kv) {
    // Insert into the in-memory RedBlackTree
    tree->insert(kv);
    currentSize++;
}

// Get a key-value pair from the memtable
KeyValueWrapper Memtable::get(const KeyValueWrapper& kv) {
    // Search only in the in-memory RedBlackTree
    return tree->getValue(kv);
}

// Scan the memtable for keys within a range
void Memtable::scan(const KeyValueWrapper& smallKey, const KeyValueWrapper& largeKey, std::set<KeyValueWrapper>& res) {
    // Scan the in-memory RedBlackTree
    tree->Scan(tree->getRoot(), smallKey, largeKey, res);
}

// Flush the memtable and return key-value pairs
std::vector<KeyValueWrapper> Memtable::flush() {
    // Get all key-value pairs from the RedBlackTree in sorted order
    std::vector<KeyValueWrapper> kvPairs;

    // Use a lambda function to collect the key-value pairs
    tree->inOrderTraversal([&kvPairs](KeyValueWrapper& kv) {
        kvPairs.push_back(kv);
    });

    // Clear the tree and reset current size
    delete tree;
    tree = new RedBlackTree();
    currentSize = 0;

    return kvPairs;
}






