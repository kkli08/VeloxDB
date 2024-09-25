//
// Created by Damian Li on 2024-08-26.
//

//
// Memtable.h
//

#ifndef MEMTABLE_H
#define MEMTABLE_H

#include "RedBlackTree.h"
#include "SstFileManager.h"
#include <filesystem>
#include <set>

namespace fs = std::filesystem;

class Memtable {
public:
    // Constructors and Destructor
    Memtable(std::shared_ptr<SSTFileManager> sstFileManager);
    explicit Memtable(int threshold, std::shared_ptr<SSTFileManager> sstFileManager);
    ~Memtable();

    // Set and get the database path
    void setPath(const fs::path& dbPath);
    fs::path getPath() const;

    // Insert a key-value pair into the memtable
    void put(const KeyValueWrapper& kv);

    // Get a key-value pair from the memtable
    KeyValueWrapper get(const KeyValueWrapper& kv);

    // Scan the memtable for keys within a range
    void scan(const KeyValueWrapper& smallKey, const KeyValueWrapper& largeKey, std::set<KeyValueWrapper>& res);

    // Flush the memtable to disk (SST file)
    void flushToDisk();

    // set sstFileManager degree
    void setSstFileManager_BTree_Degree(int degree) {sstFileManager->setDegree(degree);};

    // get current size
    int get_currentSize() const {return currentSize;};

private:
    // In-memory Red-Black Tree
    RedBlackTree* tree;

    // Threshold for flushing memtable to disk
    int memtableSize;

    // Current number of entries in the memtable
    int currentSize;

    // Database path
    fs::path dbPath;

    // SSTFileManager for flushing to disk
    std::shared_ptr<SSTFileManager> sstFileManager;
};

#endif // MEMTABLE_H



