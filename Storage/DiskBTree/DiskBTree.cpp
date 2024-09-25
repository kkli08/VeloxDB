//
// Created by damian on 9/24/24.
//

//
// DiskBTree.cpp
//

#include "DiskBTree.h"
#include <stdexcept>
#include <algorithm>

// Constructor for building a new B-tree from memtable data
DiskBTree::DiskBTree(const std::string& sstFileName, int degree, const std::vector<KeyValueWrapper>& keyValues)
    : degree(degree), pageManager(sstFileName), sstFileName(sstFileName) {
    // Build the B-tree from the sorted key-values
    buildTree(keyValues);
    // Write metadata (root offset)
    writeMetadata();
}

// Constructor for opening an existing SST file
DiskBTree::DiskBTree(const std::string& sstFileName, int degree)
    : degree(degree), pageManager(sstFileName), sstFileName(sstFileName) {
    // Read the root offset from metadata
    readMetadata();
}

// Destructor
DiskBTree::~DiskBTree() {
    // Close the page manager
    pageManager.close();
}

// Build the B-tree from sorted key-values
void DiskBTree::buildTree(const std::vector<KeyValueWrapper>& keyValues) {
    // Initialize an empty root node
    BTreeNode rootNode(true, pageManager.allocatePage());
    rootOffset = rootNode.selfOffset;

    // Insert all key-values into the B-tree
    for (const auto& kv : keyValues) {
        if (rootNode.isFull(degree)) {
            // Root is full, need to split
            BTreeNode newRoot = allocateNode(false);
            newRoot.childOffsets.push_back(rootNode.selfOffset);
            newRoot.splitChild(0, degree, this);
            rootNode = newRoot;
            rootOffset = newRoot.selfOffset;
        }
        rootNode.insertNonFull(kv, degree, this);
    }

    // Write the root node to disk
    writeNode(rootNode);
}

// Search for a key in the B-tree
KeyValueWrapper* DiskBTree::search(const KeyValueWrapper& kv) {
    BTreeNode root = readNode(rootOffset);
    return root.search(kv, this);
}

// Scan keys within a range
void DiskBTree::scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result) {
    BTreeNode root = readNode(rootOffset);
    root.scan(startKey, endKey, this, result);
}

// Get the SST file name
std::string DiskBTree::getFileName() const {
    return sstFileName;
}

// Read node from disk
DiskBTree::BTreeNode DiskBTree::readNode(uint64_t offset) {
    Page page = pageManager.readPage(offset);
    return BTreeNode::fromPage(page, offset);
}

// Write node to disk
void DiskBTree::writeNode(const BTreeNode& node) {
    Page page = node.toPage();
    pageManager.writePage(node.selfOffset, page);
}

// Allocate a new node
DiskBTree::BTreeNode DiskBTree::allocateNode(bool isLeaf) {
    uint64_t offset = pageManager.allocatePage();
    return BTreeNode(isLeaf, offset);
}

// Write metadata (root offset)
void DiskBTree::writeMetadata() {
    // Create a metadata page
    Page metadataPage(Page::PageType::SST_METADATA);
    metadataPage.setMetadata(rootOffset, 0, 0, sstFileName); // leafBegin and leafEnd can be set appropriately
    // Write the metadata page at offset 0
    pageManager.writePage(0, metadataPage);
}

// Read metadata (root offset)
void DiskBTree::readMetadata() {
    Page metadataPage = pageManager.readPage(0);
    uint64_t leafBegin, leafEnd;
    std::string fileName;
    metadataPage.getMetadata(rootOffset, leafBegin, leafEnd, fileName);
}

// BTreeNode constructor
DiskBTree::BTreeNode::BTreeNode(bool isLeaf, uint64_t offset)
    : isLeaf(isLeaf), selfOffset(offset) {}

// Check if the node is full
bool DiskBTree::BTreeNode::isFull(int degree) const {
    return keys.size() == 2 * degree - 1;
}

// Insert non-full (used during initial build)
void DiskBTree::BTreeNode::insertNonFull(const KeyValueWrapper& kv, int degree, DiskBTree* tree) {
    int i = keys.size() - 1;
    if (isLeaf) {
        // Insert the new key into the leaf node
        keys.push_back(kv);
        while (i >= 0 && kv < keys[i]) {
            keys[i + 1] = keys[i];
            --i;
        }
        keys[i + 1] = kv;
        tree->writeNode(*this);
    } else {
        // Find the child that is going to have the new key
        while (i >= 0 && kv < keys[i]) {
            --i;
        }
        ++i;
        // Read the child
        BTreeNode childNode = tree->readNode(childOffsets[i]);
        if (childNode.isFull(degree)) {
            // Split the child
            splitChild(i, degree, tree);
            if (kv > keys[i]) {
                ++i;
            }
            childNode = tree->readNode(childOffsets[i]);
        }
        childNode.insertNonFull(kv, degree, tree);
    }
}

// Split child node (used during initial build)
void DiskBTree::BTreeNode::splitChild(int idx, int degree, DiskBTree* tree) {
    BTreeNode y = tree->readNode(childOffsets[idx]);
    BTreeNode z = tree->allocateNode(y.isLeaf);

    // Move the last (degree - 1) keys from y to z
    z.keys.assign(y.keys.begin() + degree, y.keys.end());
    y.keys.resize(degree - 1);

    if (!y.isLeaf) {
        // Move the last degree child offsets from y to z
        z.childOffsets.assign(y.childOffsets.begin() + degree, y.childOffsets.end());
        y.childOffsets.resize(degree);
    }

    // Insert new child into this node
    childOffsets.insert(childOffsets.begin() + idx + 1, z.selfOffset);
    keys.insert(keys.begin() + idx, y.keys[degree - 1]);

    // Write nodes back to disk
    tree->writeNode(y);
    tree->writeNode(z);
    tree->writeNode(*this);
}

// Search for a key
KeyValueWrapper* DiskBTree::BTreeNode::search(const KeyValueWrapper& kv, DiskBTree* tree) {
    int i = 0;
    while (i < keys.size() && kv > keys[i]) {
        ++i;
    }
    if (i < keys.size() && kv == keys[i]) {
        return &keys[i];
    }
    if (isLeaf) {
        return nullptr;
    } else {
        // Read the child node from disk
        BTreeNode childNode = tree->readNode(childOffsets[i]);
        return childNode.search(kv, tree);
    }
}

// Scan keys within a range
void DiskBTree::BTreeNode::scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, DiskBTree* tree, std::vector<KeyValueWrapper>& result) {
    int i = 0;
    while (i < keys.size() && keys[i] < startKey) {
        ++i;
    }
    if (isLeaf) {
        while (i < keys.size() && keys[i] <= endKey) {
            result.push_back(keys[i]);
            ++i;
        }
    } else {
        for (; i <= keys.size(); ++i) {
            BTreeNode childNode = tree->readNode(childOffsets[i]);
            childNode.scan(startKey, endKey, tree, result);
            if (i < keys.size() && keys[i] > endKey) {
                break;
            }
        }
    }
}

// Convert node to page
Page DiskBTree::BTreeNode::toPage() const {
    Page page(isLeaf ? Page::PageType::LEAF_NODE : Page::PageType::INTERNAL_NODE);

    if (isLeaf) {
        for (const auto& kv : keys) {
            page.addLeafEntry(kv);
        }
    } else {
        if (childOffsets.size() != keys.size() + 1) {
            throw std::runtime_error("Inconsistent number of child offsets and keys in internal node");
        }
        for (const auto& childOffset : childOffsets) {
            page.addChildOffset(childOffset);
        }
        for (const auto& key : keys) {
            page.addKey(key);
        }
    }
    return page;
}

// Create node from page
DiskBTree::BTreeNode DiskBTree::BTreeNode::fromPage(const Page& page, uint64_t offset) {
    bool isLeaf = (page.getPageType() == Page::PageType::LEAF_NODE);
    BTreeNode node(isLeaf, offset);

    if (isLeaf) {
        node.keys = page.getLeafEntries();
    } else {
        node.childOffsets = page.getChildOffsets();
        node.keys = page.getInternalKeys();
    }
    return node;
}


