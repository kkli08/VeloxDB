//
// Created by Damian Li on 2024-08-26.
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
    Memtable();
    Memtable(int threshold);
    ~Memtable();

    void set_path(fs::path);
    fs::path get_path();

    void Scan(const KeyValueWrapper& small_key, const KeyValueWrapper& large_key, std::set<KeyValueWrapper>& res);
    KeyValueWrapper put(const KeyValueWrapper& kv);
    KeyValueWrapper get(const KeyValueWrapper& kv);

private:
    RedBlackTree* tree;
    int memtable_size;
    int current_size = 0;
    fs::path path;
    SstFileManager sstFileManager;
};

#endif // MEMTABLE_H

