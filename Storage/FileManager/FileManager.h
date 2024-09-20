//
// Created by Damian Li on 2024-09-08.
//

#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "KeyValue.h"
#include <filesystem>
#include <vector>
#include <fstream>
#include <string>
#include <RedBlackTree.h>

namespace fs = std::filesystem;

struct FlushSSTInfo {
    std::string fileName;
    KeyValueWrapper smallest_key;
    KeyValueWrapper largest_key;
};

struct SSTHeader {
    uint32_t num_key_values;   // Number of KeyValue pairs in the file
    uint32_t header_checksum;  // Checksum for the header
    uint32_t calculateChecksum() const;

    void serialize(std::ofstream& file) const;
    static SSTHeader deserialize(std::ifstream& file);
};

class FileManager {
public:
    FileManager();
    explicit FileManager(fs::path directory);

    FlushSSTInfo flushToDisk(const std::vector<KeyValueWrapper>& kv_pairs);
    RedBlackTree* loadFromDisk(const std::string& sst_filename);

    std::string generateSstFilename();
    void setDirectory(const fs::path& path);
    fs::path getDirectory() const;
    int increaseFileCounter() {return ++sstFileCounter - 1;};

private:
    fs::path directory;
    int sstFileCounter = 0;
    SSTHeader sstHeader;
};

#endif // FILEMANAGER_H





