//
// Created by damian on 9/24/24.
//

//
// SSTFileManager.h
//

#ifndef SST_FILE_MANAGER_H
#define SST_FILE_MANAGER_H

#include "DiskBTree.h"
#include <string>
#include <vector>
#include <memory>

class SSTFileManager {
public:
    // Constructor
    SSTFileManager(const std::string& dbDirectory, int degree);

    // Flush memtable to SST file
    void flushMemtable(const std::vector<KeyValueWrapper>& keyValues);

    // Search for a key across all SST files
    KeyValueWrapper* search(const KeyValueWrapper& kv);

    // Scan keys within a range across all SST files
    void scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result);

private:
    // Directory where SST files are stored
    std::string dbDirectory;

    // B+ tree degree
    int degree;

    // List of SST files (DiskBTree instances)
    std::vector<std::shared_ptr<DiskBTree>> sstFiles;

    // Method to generate new SST file names
    std::string generateSSTFileName();

    // Other private methods...
};

#endif // SST_FILE_MANAGER_H




