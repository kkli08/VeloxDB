//
// Created by damian on 9/24/24.
//

//
// DiskBTree.h
//

#ifndef DISKBTREE_H
#define DISKBTREE_H

#include "Page.h"
#include "PageManager.h"
#include "KeyValue.h"
#include <string>
#include <vector>

class DiskBTree {
public:
    // Constructor
    DiskBTree(int degree, const std::string& fileName);

    // Destructor
    ~DiskBTree();

    // Insert a key-value pair into the B+ Tree
    void insert(const KeyValueWrapper& kv);

    // Search for a key in the B+ Tree
    KeyValueWrapper* search(const KeyValueWrapper& kv);

    // Traverse and print the B+ Tree
    void traverse();

    void scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result);


private:
    struct BTreeNode {
        // Node attributes
        bool isLeaf;
        uint16_t numKeys;
        std::vector<KeyValueWrapper> keys;
        std::vector<uint64_t> childOffsets; // Offsets to child pages
        uint64_t nextLeafOffset;            // For leaf nodes

        // Offset of this node in the file
        uint64_t selfOffset;

        // Constructor
        BTreeNode(bool leaf, uint64_t offset);

        // Methods
        void insertNonFull(const KeyValueWrapper& kv, int degree, DiskBTree* tree);
        void splitChild(int idx, int degree, DiskBTree* tree);
        KeyValueWrapper* search(const KeyValueWrapper& kv, DiskBTree* tree);
        void traverse(DiskBTree* tree);
        void scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, DiskBTree* tree, std::vector<KeyValueWrapper>& result);

        // Serialization and Deserialization
        Page toPage() const;
        static BTreeNode fromPage(const Page& page, uint64_t offset);
    };

    int degree;               // Minimum degree
    uint64_t rootOffset;      // Offset of the root node
    PageManager pageManager;  // Manages pages on disk

    // Helper methods
    BTreeNode readNode(uint64_t offset);
    void writeNode(const BTreeNode& node);
};

#endif // DISKBTREE_H


