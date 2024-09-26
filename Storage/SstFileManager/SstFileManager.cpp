//
// Created by damian on 9/24/24.
//

//
// SSTFileManager.cpp
//

#include "SstFileManager.h"
#include <filesystem>
#include <chrono>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

// Constructor
SSTFileManager::SSTFileManager(const std::string& dbDirectory, int degree)
    : dbDirectory(dbDirectory), degree(degree) {
    // std::cout << "SSTFileManager initialized with directory: " << dbDirectory << " and degree: " << degree << std::endl;
    // Ensure the database directory exists
    if (!fs::exists(dbDirectory)) {
        fs::create_directories(dbDirectory);
    }
    // Initialize by loading existing SST files if any
    for (const auto& entry : std::filesystem::directory_iterator(dbDirectory)) {
        if (entry.path().extension() == ".sst") {
            auto sst = std::make_shared<DiskBTree>(entry.path().string(), degree);
            sstFiles.push_back(sst);
        }
    }
}

// Flush memtable to SST file
void SSTFileManager::flushMemtable(const std::vector<KeyValueWrapper>& keyValues) {
    // // Sort the keyValues
    // auto sortedKeyValues = keyValues;
    // std::sort(sortedKeyValues.begin(), sortedKeyValues.end());

    // no data to flush
    if(keyValues.empty()) return;

    // Generate a new SST file name
    std::string sstFileName = generateSSTFileName();

    // Create a new DiskBTree instance for the SST file
    auto sst = std::make_shared<DiskBTree>(sstFileName, degree, keyValues);

    // Add the new SST to the list
    sstFiles.push_back(sst);
    // std::cout << "Created SST file: " << sstFileName << " with " << keyValues.size() << " key-value pairs." << std::endl;
}

// Generate a new SST file name
std::string SSTFileManager::generateSSTFileName() {
    // Use timestamp for uniqueness
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    std::ostringstream oss;
    oss << dbDirectory << "/sst_" << now << ".sst";
    return oss.str();
}

// Search for a key across all SST files
KeyValueWrapper* SSTFileManager::search(const KeyValueWrapper& kv) {
    for (auto it = sstFiles.rbegin(); it != sstFiles.rend(); ++it) {
        KeyValueWrapper* result = (*it)->search(kv);
        if (result != nullptr) {
            return result;
        }
    }
    return nullptr;
}

// Scan keys within a range across all SST files
void SSTFileManager::scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result) {
    std::set<KeyValueWrapper> tempResultSet;  // Using set to handle duplicates automatically based on key comparison

    // Scan each SST file from newest to oldest
    std::cout << "Scanning across " << sstFiles.size() << " SST files." << std::endl;
    for (auto it = sstFiles.rbegin(); it != sstFiles.rend(); ++it) {
        std::vector<KeyValueWrapper> tempResult;
        (*it)->scan(startKey, endKey, tempResult);

        // Insert all key-value pairs into the tempResultSet, which handles duplicates automatically
        for (const auto& kv : tempResult) {
            tempResultSet.insert(kv);
        }
    }

    // Convert set back to vector (as required by your API)
    result.assign(tempResultSet.begin(), tempResultSet.end());
    std::cout << "Scan completed with " << result.size() << " key-value pairs found." << std::endl;
}

void SSTFileManager::setBufferPoolParameters(size_t capacity, EvictionPolicy policy) {
    bufferPoolCapacity = capacity;
    bufferPoolPolicy = policy;

    // Update existing DiskBTrees
    for (auto& sst : sstFiles) {
        sst->setBufferPoolParameters(capacity, policy);
    }
}





