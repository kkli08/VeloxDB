//
// Created by damian on 9/24/24.
//

#include "SstFileManager.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>

SstFileManager::SstFileManager() : directory("defaultDB") {
    if (!fs::exists(directory)) {
        fs::create_directories(directory);
    }
    loadSSTFiles();
}

SstFileManager::SstFileManager(fs::path dir) : directory(std::move(dir)) {
    if (!fs::exists(directory)) {
        fs::create_directories(directory);
    }
    loadSSTFiles();
}

void SstFileManager::setDirectory(const fs::path& path) {
    directory = path;
    if (!fs::exists(directory)) {
        fs::create_directories(directory);
    }
    loadSSTFiles();
}

fs::path SstFileManager::getDirectory() const {
    return directory;
}

std::string SstFileManager::generateSstFilename() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << "sst_" << sstFileCounter << "_";
    ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S") << ".sst";
    sstFileCounter++;
    return ss.str();
}

FlushSSTInfo SstFileManager::flushToDisk(const std::vector<KeyValueWrapper>& kv_pairs) {
    FlushSSTInfo flushInfo;
    if (kv_pairs.empty()) {
        return flushInfo;
    }

    flushInfo.smallest_key = kv_pairs.front();
    flushInfo.largest_key = kv_pairs.back();

    flushInfo.fileName = generateSstFilename();
    std::string sstFilePath = (directory / flushInfo.fileName).string();
    int degree = 3; // Adjust as needed

    // Create a new DiskBTree
    auto diskBTree = std::make_shared<DiskBTree>(degree, sstFilePath);

    // Insert kv_pairs into the DiskBTree
    for (const auto& kv : kv_pairs) {
        diskBTree->insert(kv);
    }

    // Save the DiskBTree instance
    sstFiles.push_back(diskBTree);

    return flushInfo;
}

void SstFileManager::loadSSTFiles() {
    sstFiles.clear();

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".sst") {
            std::string sstFilePath = entry.path().string();
            int degree = 3; // Should match the degree used when creating the tree

            // Create a DiskBTree instance for the SST file
            auto diskBTree = std::make_shared<DiskBTree>(degree, sstFilePath);

            // Save the DiskBTree instance
            sstFiles.push_back(diskBTree);

            // Update sstFileCounter
            std::string filename = entry.path().filename().string();
            size_t pos1 = filename.find('_');
            size_t pos2 = filename.find('_', pos1 + 1);
            if (pos1 != std::string::npos && pos2 != std::string::npos) {
                int counter = std::stoi(filename.substr(pos1 + 1, pos2 - pos1 - 1));
                if (counter >= sstFileCounter) {
                    sstFileCounter = counter + 1;
                }
            }
        }
    }
}

KeyValueWrapper SstFileManager::search(const KeyValueWrapper& key) {
    // Search in reverse order to check the most recent SST files first
    for (auto it = sstFiles.rbegin(); it != sstFiles.rend(); ++it) {
        KeyValueWrapper* result = (*it)->search(key);
        if (result != nullptr && !result->isEmpty()) {
            return *result;
        }
    }
    return KeyValueWrapper(); // Return empty if not found
}

void SstFileManager::scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result) {
    for (const auto& sst : sstFiles) {
        std::vector<KeyValueWrapper> sstResult;
        sst->scan(startKey, endKey, sstResult);
        result.insert(result.end(), sstResult.begin(), sstResult.end());
    }

    // Remove duplicates and sort
    std::sort(result.begin(), result.end(), [](const KeyValueWrapper& a, const KeyValueWrapper& b) {
        return a < b;
    });
    result.erase(std::unique(result.begin(), result.end(), [](const KeyValueWrapper& a, const KeyValueWrapper& b) {
        return !(a < b) && !(b < a);
    }), result.end());
}

