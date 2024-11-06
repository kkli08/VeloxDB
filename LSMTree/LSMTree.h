// LSMTree.h
#ifndef LSMTREE_H
#define LSMTREE_H

#include "Memtable.h"
#include "DiskBTree.h"
#include <vector>
#include <string>
#include <memory>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class LSMTree {
public:
    // Constructor with optional memtable size (default to 1000)
    LSMTree(size_t memtableSize = 1000, const std::string& dbPath = "defaultDB");

    // Destructor
    ~LSMTree();

    // Save the state of the LSM tree to a .lsm file
    void saveState();

    // Load the state of the LSM tree from a .lsm file
    void loadState();

    // Get the number of levels in the LSM tree
    size_t getNumLevels() const;

    // Set the database path
    void setDBPath(const std::string& path);

    // Get the database path
    std::string getDBPath() const;

    // Insert a key-value pair into the LSM tree
    void put(const KeyValueWrapper& kv);

    // Search for a key-value pair in the LSM tree
    KeyValueWrapper get(const KeyValueWrapper& kv);

    // Scan method
    void scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result);

    // print LSM-Tree structure
    void printTree() const;
private:
    // Level 0 is always the in-memory memtable
    std::unique_ptr<Memtable> memtable; // Level 0

    // Levels 1 and above consist of DiskBTrees (SSTables)
    // Each level holds at most one SSTable
    std::vector<std::shared_ptr<DiskBTree>> levels; // Levels 1+

    size_t fixedSizeRatio = 2;

    // Level capacities (maximum number of key-value pairs per level)
    std::vector<size_t> levelMaxSizes;

    // Path to the .lsm file and database directory
    fs::path dbPath;
    fs::path lsmFilePath;

    // Helper methods
    void initializeLSM();

    // Handle flushing memtable to Level 1
    void flushMemtableToLevel1();

    // Merge SSTables when a level exceeds its capacity
    void mergeLevels(int level, const std::shared_ptr<DiskBTree>& sstToMerge);

    // Merge two SSTables into a new SSTable
    void mergeSSTables(const std::shared_ptr<DiskBTree>& sst1,
                       const std::shared_ptr<DiskBTree>& sst2,
                       const std::string& outputSSTableFileName,
                       std::vector<KeyValueWrapper>& leafPageSmallestKeys,
                       int& numberOfPages,
                       int& totalKvs);

    // Generate unique SSTable file names
    std::string generateSSTableFileName(int level);

    // Disable copy and assignment
    LSMTree(const LSMTree&) = delete;
    LSMTree& operator=(const LSMTree&) = delete;
};

#endif // LSMTREE_H
