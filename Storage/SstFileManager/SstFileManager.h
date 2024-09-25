//
// Created by damian on 9/24/24.
//

#ifndef SSTFILEMANAGER_H
#define SSTFILEMANAGER_H

#include "DiskBTree.h"
#include "KeyValue.h"
#include <filesystem>
#include <vector>
#include <string>
#include <memory>

namespace fs = std::filesystem;

struct FlushSSTInfo {
    std::string fileName;
    KeyValueWrapper smallest_key;
    KeyValueWrapper largest_key;
};

class SstFileManager {
public:
    // Constructors
    SstFileManager();
    explicit SstFileManager(fs::path directory);

    // Flush data to disk as an SST file
    FlushSSTInfo flushToDisk(const std::vector<KeyValueWrapper>& kv_pairs);

    // Search for a key in all SST files
    KeyValueWrapper search(const KeyValueWrapper& key);

    // Scan over a range of keys in all SST files
    void scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result);

    // Set and get directory for SST files
    void setDirectory(const fs::path& path);
    fs::path getDirectory() const;

    // Generate a unique SST file name
    std::string generateSstFilename();

private:
    fs::path directory;
    int sstFileCounter = 0;
    std::vector<std::shared_ptr<DiskBTree>> sstFiles;

    // Load existing SST files from disk
    void loadSSTFiles();
};

#endif // SSTFILEMANAGER_H

