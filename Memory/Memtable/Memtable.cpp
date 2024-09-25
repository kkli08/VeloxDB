//
// Created by Damian Li on 2024-08-26.
//

//
// Memtable.cpp
//

#include "Memtable.h"
#include <algorithm>

// Default Constructor
Memtable::Memtable()
    : memtableSize(10000), // Default threshold
      currentSize(0),
      tree(new RedBlackTree()),
      dbPath("defaultDB"),
      sstFileManager("defaultDB", /*degree=*/3) {
    // Ensure the database directory exists
    if (!fs::exists(dbPath)) {
        fs::create_directories(dbPath);
    }
}

// Constructor with threshold
Memtable::Memtable(int threshold)
    : memtableSize(threshold),
      currentSize(0),
      tree(new RedBlackTree()),
      dbPath("defaultDB"),
      sstFileManager("defaultDB", /*degree=*/3) {
    // Ensure the database directory exists
    if (!fs::exists(dbPath)) {
        fs::create_directories(dbPath);
    }
}

// Destructor
Memtable::~Memtable() {
    delete tree;
}

// Set the database path
void Memtable::setPath(const fs::path& _dbPath) {
    dbPath = _dbPath;
    if (!fs::exists(dbPath)) {
        fs::create_directories(dbPath);
    }
    // Update the SSTFileManager's directory
    sstFileManager = SSTFileManager(dbPath.string(), /*degree=*/3);
}

// Get the database path
fs::path Memtable::getPath() const {
    return dbPath;
}

// Insert a key-value pair into the memtable
void Memtable::put(const KeyValueWrapper& kv) {
    if(currentSize <= memtableSize) {
        // Insert into the in-memory RedBlackTree
        tree->insert(kv);
        currentSize++;
    }

    // Check if the memtable size limit is reached
    if (currentSize > memtableSize) {
        // Flush to SST file
        flushToDisk();

        // Reset the memtable
        delete tree;
        tree = new RedBlackTree();
        currentSize = 0;
        // Insert into the in-memory RedBlackTree
        tree->insert(kv);
        currentSize++;
    }
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

// **Updated flushToDisk method**
void Memtable::flushToDisk() {
    // Get all key-value pairs from the RedBlackTree in sorted order
    std::vector<KeyValueWrapper> kvPairs;

    // Use a lambda function to collect the key-value pairs
    tree->inOrderTraversal([&kvPairs](KeyValueWrapper& kv) {
        kvPairs.push_back(kv);
    });

    // Flush to SST file using SSTFileManager
    sstFileManager.flushMemtable(kvPairs);
}





