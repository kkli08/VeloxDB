//
// Created by Damian Li on 2024-08-26.
//

#include "Memtable.h"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <string>
#include <filesystem> // c++17 features
#include <sstream>
#include <iostream>
#include <cstring>

using namespace std;
namespace fs = std::filesystem;

// Constructor
Memtable::Memtable(int threshold) {
    memtable_size = threshold;
    current_size = 0;
    tree = new RedBlackTree();
    path = fs::path("defaultDB");

}

Memtable::Memtable() {
    memtable_size = 1e4;
    current_size = 0;
    tree = new RedBlackTree();
}
// Destructor
Memtable::~Memtable() {
    delete tree;
}

FlushSSTInfo Memtable::put(const KeyValueWrapper& kv) {
    FlushSSTInfo info;

    // Check if the memtable size limit is not reached
    if (current_size < memtable_size) {
        // Insert the key-value pair into the tree
        tree->insert(kv);
        current_size++;
    } else if (current_size == memtable_size) {
        // If the tree is full, check if the key exists to avoid unnecessary flush
        if (!tree->search(kv)) {
            if (!fs::exists(path)) {
                fs::create_directories(path);  // Ensure the directory exists
            }
            // Flush the current tree to disk and reset the size
            info = file_manager.flushToDisk(tree->inOrderFlushToSst());
            current_size = 0;

            // Reallocate memory for the RedBlackTree
            delete tree;
            tree = new RedBlackTree();
        }

        // Insert the new key-value pair
        tree->insert(kv);
        current_size++;
    }

    return info;
}


KeyValueWrapper Memtable::get(const KeyValueWrapper& kv) {
    return tree->getValue(kv);
}

void Memtable::set_path(fs::path _path) {
    // Check if the directory exists
    if (!fs::exists(_path)) {
        // If the directory does not exist, create it
        fs::create_directories(_path);
    }
    path = _path;
    file_manager.setDirectory(path);
}
fs::path Memtable::get_path() {
    return path;
}



std::string Memtable::generateSstFilename() {
    // Get the current SST file size count
    int sstId = getSSTFileSize();

    // Generate the current timestamp
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y_%m_%d_%H%M");

    // Combine the SST file size and the timestamp to create the filename
    std::string filename = std::to_string(sstId) + "_" + ss.str() + ".sst";

    // Increase the SST file size count for the next filename
    increaseSSTFileSize();

    return filename;
}

// scan the tree and insert the kv-pairs<k,v> into res where small_key <= k && k <= large_key
void Memtable::Scan(KeyValueWrapper small_key, KeyValueWrapper large_key, set<KeyValueWrapper>& res) {
    tree->Scan(tree->getRoot(), small_key, large_key, res);
}

