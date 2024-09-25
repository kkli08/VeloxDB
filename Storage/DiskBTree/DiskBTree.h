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

class DiskBTree {
public:
    // Constructor for building a new B-tree from memtable data
    DiskBTree(const std::string& sstFileName, int degree, const std::vector<KeyValueWrapper>& keyValues);

    // Constructor for opening an existing SST file
    DiskBTree(const std::string& sstFileName, int degree);

    // Destructor
    ~DiskBTree();

    // Search for a key in the B-tree
    KeyValueWrapper* search(const KeyValueWrapper& kv);

    // Scan keys within a range
    void scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result);

    // Get the SST file name
    std::string getFileName() const;

private:
    // B-tree degree
    int degree;

    // PageManager for disk I/O
    PageManager pageManager;

    // Offset of the root node
    uint64_t rootOffset;

    // File name of the SST file
    std::string sstFileName;

    // B-tree node definition
    struct BTreeNode {
        bool isLeaf;
        std::vector<KeyValueWrapper> keys;
        std::vector<uint64_t> childOffsets; // For internal nodes
        uint64_t selfOffset;                // Offset of this node on disk

        // Constructor
        BTreeNode(bool isLeaf, uint64_t offset);

        // Insert non-full (used during initial build)
        void insertNonFull(const KeyValueWrapper& kv, int degree, DiskBTree* tree);

        // Split child node (used during initial build)
        void splitChild(int idx, int degree, DiskBTree* tree);

        // Search for a key
        KeyValueWrapper* search(const KeyValueWrapper& kv, DiskBTree* tree);

        // Scan keys within a range
        void scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, DiskBTree* tree, std::vector<KeyValueWrapper>& result);

        // Convert node to page
        Page toPage() const;

        // Create node from page
        static BTreeNode fromPage(const Page& page, uint64_t offset);

        // Check if the node is full
        bool isFull(int degree) const;
    };

    // Read node from disk
    BTreeNode readNode(uint64_t offset);

    // Write node to disk
    void writeNode(const BTreeNode& node);

    // Allocate a new node
    BTreeNode allocateNode(bool isLeaf);

    // Build B-tree from sorted key-values
    void buildTree(const std::vector<KeyValueWrapper>& keyValues);

    // Methods to read and write metadata (root offset)
    void writeMetadata();
    void readMetadata();

    // Other private methods...
};

#endif // DISK_BTREE_H





