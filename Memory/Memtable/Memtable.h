//
// Created by Damian Li on 2024-08-26.
//

// Memtable.h
#ifndef MEMTABLE_H
#define MEMTABLE_H

#include "RedBlackTree.h"
#include <filesystem>
#include <set>
#include <vector>

namespace fs = std::filesystem;

class Memtable {
public:
    // Constructors and Destructor
    Memtable();
    explicit Memtable(int threshold);
    ~Memtable();

    // Insert a key-value pair into the memtable
    void put(const KeyValueWrapper& kv);

    // Get a key-value pair from the memtable
    KeyValueWrapper get(const KeyValueWrapper& kv);

    // Scan the memtable for keys within a range
    void scan(const KeyValueWrapper& smallKey, const KeyValueWrapper& largeKey, std::set<KeyValueWrapper>& res);

    // Get current size
    int getCurrentSize() const { return currentSize; }

    // Set and get the memtable threshold
    void setThreshold(int threshold) { memtableSize = threshold; }
    int getThreshold() const { return memtableSize; }

    // Flush the memtable and return key-value pairs
    std::vector<KeyValueWrapper> flush();

private:
    // In-memory Red-Black Tree
    RedBlackTree* tree;

    // Threshold for flushing memtable
    int memtableSize;

    // Current number of entries in the memtable
    int currentSize;
};

#endif // MEMTABLE_H




