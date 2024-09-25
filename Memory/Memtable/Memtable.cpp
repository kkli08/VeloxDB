//
// Created by Damian Li on 2024-08-26.
//

#include "Memtable.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Constructor
Memtable::Memtable(int threshold) : memtable_size(threshold) {
    current_size = 0;
    tree = new RedBlackTree();
    path = fs::path("defaultDB");
    sstFileManager.setDirectory(path);
}

Memtable::Memtable() {
    memtable_size = 10000; // Default threshold
    current_size = 0;
    tree = new RedBlackTree();
    path = fs::path("defaultDB");
    sstFileManager.setDirectory(path);
}

// Destructor
Memtable::~Memtable() {
    delete tree;
}

KeyValueWrapper Memtable::put(const KeyValueWrapper& kv) {
    // Insert into memtable
    tree->insert(kv);
    current_size++;

    // Check if memtable size limit is reached
    if (current_size >= memtable_size) {
        // Flush to SST
        std::vector<KeyValueWrapper> kv_pairs = tree->inOrderFlushToSst();
        sstFileManager.flushToDisk(kv_pairs);

        // Reset memtable
        delete tree;
        tree = new RedBlackTree();
        current_size = 0;
    }

    return kv;
}

KeyValueWrapper Memtable::get(const KeyValueWrapper& kv) {
    // First, check in memtable
    KeyValueWrapper result = tree->getValue(kv);
    if (!result.isEmpty()) {
        return result;
    }

    // // Then, check in SST files
    // result = sstFileManager.search(kv);
    return result;
}

void Memtable::set_path(fs::path _path) {
    if (!fs::exists(_path)) {
        fs::create_directories(_path);
    }
    path = _path;
    sstFileManager.setDirectory(path);
}

fs::path Memtable::get_path() {
    return path;
}

void Memtable::Scan(const KeyValueWrapper& small_key, const KeyValueWrapper& large_key, std::set<KeyValueWrapper>& res) {
    // Scan memtable
    tree->Scan(tree->getRoot(), small_key, large_key, res);

    // Scan SST files
    std::vector<KeyValueWrapper> sstResult;
    sstFileManager.scan(small_key, large_key, sstResult);

    // Insert into result set
    res.insert(sstResult.begin(), sstResult.end());
}


