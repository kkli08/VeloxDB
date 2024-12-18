// DiskBTree.h

#ifndef DISK_BTREE_H
#define DISK_BTREE_H

#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include "KeyValue.h"
#include "PageManager.h"
#include "BloomFilter.h"

class DiskBTree {
public:
    // Constructor for building a new B+ tree from memtable data
    DiskBTree(const std::string& sstFileName, const std::vector<KeyValueWrapper>& keyValues, size_t pageSize = 4096);

    // Constructor for opening an existing SST file
    DiskBTree(const std::string& sstFileName);

    // New constructor for building a B+ tree from existing leaf pages
    DiskBTree(const std::string& sstFileName, const std::string& leafsFileName, const std::vector<KeyValueWrapper>& leafPageSmallestKeys, int numOfPages, int totalKvs);

    // Destructor
    ~DiskBTree();

    // Get the SST file name
    std::string getFileName() const;

    // Set buffer pool parameters
    void setBufferPoolParameters(size_t capacity, EvictionPolicy policy);

    // Get cache hit count
    long long getCacheHit() const;

    // Search for a key in the B+ tree
    KeyValueWrapper* search(const KeyValueWrapper& kv);

    // Scan keys within a range
    void scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result);

    // PageManager for disk I/O
    std::shared_ptr<PageManager> pageManager;

    // Leaf node begin and end offsets
    uint64_t getLeafBeginOffset() const { return leafBeginOffset; };
    uint64_t getLeafEndOffset() const { return leafEndOffset; };

    // Method to get the number of key-value pairs
    size_t getNumberOfKeyValues() const { return totalKeyValueCount; }

    // update file name when merge to a new level
    void updateSstFileName(const std::string &newLevelFilename) {
        sstFileName = newLevelFilename;
        pageManager->close();
        pageManager = std::make_shared<PageManager>(sstFileName);
    };

    std::string getSstFilename() const { return sstFileName; };

    // print all key value pair from disk
    void printKVs() const;

private:
    // Offset of the root node
    uint64_t rootOffset;

    // Leaf node begin and end offsets
    uint64_t leafBeginOffset;
    uint64_t leafEndOffset;

    size_t totalKeyValueCount;
    // File name of the SST file
    std::string sstFileName;

    // Page size
    size_t pageSize = 4096;

    // Degree and height of the B+ tree
    size_t degree;
    size_t height;

    // Fields used during SST building
    // BTreeNode struct representing different types of nodes
    struct BTreeNode {
        bool isLeaf;
        std::vector<KeyValueWrapper> keys;
        std::vector<BTreeNode*> children;        // For internal nodes
        std::vector<size_t> leafPageIndices;     // For nodes pointing to leaf pages

        uint64_t offset; // Offset of the node in the SST file

        BTreeNode(bool leaf) : isLeaf(leaf), offset(0) {}
    };

    // Vector of leaf pages (used in the original constructor)
    std::vector<Page> leafPages;

    // Vector of smallest keys from each leaf page
    std::vector<KeyValueWrapper> leafPageSmallestKeys;

    // Root node of the tree
    BTreeNode* root;

    // For memory management
    std::vector<BTreeNode*> allNodes; // To keep track of all nodes for deletion

    // Levels of the tree, from leaf level upwards
    std::vector<std::vector<BTreeNode*>> levels;

    // Method to split input keyValues into leaf pages
    void splitInputPairs(const std::vector<KeyValueWrapper>& keyValues);

    // Method to compute degree and height
    void computeDegreeAndHeight();

    // New method to compute degree and height from leaf keys
    void computeDegreeAndHeightFromLeafKeys(const std::vector<KeyValueWrapper>& leafPageSmallestKeys);

    // Method to build the tree
    void buildTree();

    // New method to build the tree from leaf page keys and offsets
    void buildTreeFromLeafPageKeys(const std::vector<KeyValueWrapper>& leafPageSmallestKeys, const std::vector<uint64_t>& leafPageOffsets);

    // Method to write the tree into the SST file
    void writeTreeToSST();

    // New method to write the tree into the SST file using leaf page offsets
    void writeTreeToSSTWithLeafOffsets(const std::vector<uint64_t>& leafPageOffsets);


};

#endif // DISK_BTREE_H
