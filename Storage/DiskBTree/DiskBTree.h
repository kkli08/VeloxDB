//
// Created by damian on 9/24/24.
//

//
// DiskBTree.h
//

#ifndef DISK_BTREE_H
#define DISK_BTREE_H

#include "PageManager.h"
#include "Page.h"
#include "KeyValue.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class DiskBTree {
public:
    // Constructor for building a new B+ tree from memtable data
    DiskBTree(const std::string& sstFileName, int degree, const std::vector<KeyValueWrapper>& keyValues);

    // Constructor for opening an existing SST file
    DiskBTree(const std::string& sstFileName, int degree);

    // Destructor
    ~DiskBTree();

    // Search for a key in the B+ tree
    KeyValueWrapper* search(const KeyValueWrapper& kv);

    // Scan keys within a range
    void scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result);

    // Get the SST file name
    std::string getFileName() const;

private:
    // B+ tree degree
    int degree;

    // PageManager for disk I/O
    PageManager pageManager;

    // Offset of the root node
    uint64_t rootOffset;

    // File name of the SST file
    std::string sstFileName;

    // In-memory representation of a node during construction
    struct BTreeNode {
        bool isLeaf;
        std::vector<KeyValueWrapper> keys;
        std::vector<std::shared_ptr<BTreeNode>> children; // For internal nodes
        uint64_t selfOffset;                              // Offset of this node on disk

        // Constructor
        BTreeNode(bool isLeaf);

        // Methods used during tree construction
        void insertNonFull(const KeyValueWrapper& kv, int degree);
        void splitChild(int idx, int degree);

        // Method to write the node and its children to disk
        void writeNode(DiskBTree* tree);

        // Set the offset after writing to disk
        void setOffset(uint64_t offset);
    };

    // Build B+ tree from sorted key-values
    std::shared_ptr<BTreeNode> buildTree(const std::vector<KeyValueWrapper>& keyValues);

    // Methods for search and scan
    KeyValueWrapper* searchInNode(uint64_t nodeOffset, const KeyValueWrapper& kv);
    void scanInNode(uint64_t nodeOffset, const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result);

    // Other private methods...
};

#endif // DISK_BTREE_H







