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
    std::vector<KeyValueWrapper> tempResult;
    for (const auto& sst : sstFiles) {
        sst->scan(startKey, endKey, tempResult);
    }

    // Remove duplicates and resolve conflicts
    std::sort(tempResult.begin(), tempResult.end());
    auto last = std::unique(tempResult.begin(), tempResult.end());
    tempResult.erase(last, tempResult.end());

    result = std::move(tempResult);
}





