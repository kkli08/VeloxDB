//
// Created by damian on 9/24/24.
//

//
// DiskBTree.cpp
//

#include "DiskBTree.h"
#include <iostream>
#include <stdexcept>

// BTreeNode constructor
DiskBTree::BTreeNode::BTreeNode(bool leaf, uint64_t offset)
    : isLeaf(leaf), numKeys(0), nextLeafOffset(0), selfOffset(offset) {}

// Convert node to page
Page DiskBTree::BTreeNode::toPage() const {
    Page::PageType pageType = isLeaf ? Page::PageType::LEAF_NODE : Page::PageType::INTERNAL_NODE;
    Page page(pageType);

    if (isLeaf) {
        // For leaf nodes
        for (const auto& kv : keys) {
            page.addLeafEntry(kv);
        }
        page.setNextLeafOffset(nextLeafOffset);
    } else {
        // For internal nodes
        // Ensure the number of child offsets is one more than the number of keys
        if (childOffsets.size() != keys.size() + 1) {
            throw std::runtime_error("Inconsistent number of child offsets and keys in internal node");
        }

        // Add child offsets
        for (const auto& childOffset : childOffsets) {
            page.addChildOffset(childOffset);
        }

        // Add keys
        for (const auto& key : keys) {
            page.addKey(key);
        }
    }

    return page;
}


// Create node from page
DiskBTree::BTreeNode DiskBTree::BTreeNode::fromPage(const Page& page, uint64_t offset) {
    bool leaf = (page.getPageType() == Page::PageType::LEAF_NODE);
    BTreeNode node(leaf, offset);

    if (leaf) {
        node.keys = page.getLeafEntries();
        node.nextLeafOffset = page.getNextLeafOffset();
    } else {
        node.keys = page.getInternalKeys();
        node.childOffsets = page.getChildOffsets();
    }

    node.numKeys = static_cast<uint16_t>(node.keys.size());
    return node;
}

// DiskBTree constructor
DiskBTree::DiskBTree(int degree, const std::string& fileName)
    : degree(degree), pageManager(fileName) {
    // Check if the file is empty (new tree) or existing
    if (pageManager.getEOFOffset() == 0) {
        // Create a new root node
        uint64_t rootPageOffset = pageManager.allocatePage();
        BTreeNode rootNode(true, rootPageOffset);
        writeNode(rootNode);
        rootOffset = rootPageOffset;

        // Write SST metadata
        Page metadataPage(Page::PageType::SST_METADATA);
        metadataPage.setMetadata(rootOffset, 0, 0, fileName);
        pageManager.writePage(0, metadataPage);
    } else {
        // Load root offset from SST metadata
        Page metadataPage = pageManager.readPage(0);
        if (metadataPage.getPageType() == Page::PageType::SST_METADATA) {
            uint64_t leafBegin, leafEnd;
            std::string fileName;
            metadataPage.getMetadata(rootOffset, leafBegin, leafEnd, fileName);
        } else {
            throw std::runtime_error("Invalid SST file: Missing metadata page");
        }
    }
}


// DiskBTree destructor
DiskBTree::~DiskBTree() {
    // Close the page manager
    pageManager.close();
}

// Insert a key-value pair
void DiskBTree::insert(const KeyValueWrapper& kv) {
    BTreeNode rootNode = readNode(rootOffset);

    if (rootNode.numKeys == 2 * degree - 1) {
        // Root is full, need to split
        uint64_t newRootOffset = pageManager.allocatePage();
        BTreeNode newRoot(false, newRootOffset);
        newRoot.childOffsets.push_back(rootOffset);

        newRoot.splitChild(0, degree, this);
        rootOffset = newRootOffset;
        writeNode(newRoot);

        int i = 0;
        if (newRoot.keys[0] < kv)
            i++;
        BTreeNode childNode = readNode(newRoot.childOffsets[i]);
        childNode.insertNonFull(kv, degree, this);
    } else {
        rootNode.insertNonFull(kv, degree, this);
    }
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

// BTreeNode methods

void DiskBTree::BTreeNode::insertNonFull(const KeyValueWrapper& kv, int degree, DiskBTree* tree) {
    int i = numKeys - 1;

    if (isLeaf) {
        // Insert into leaf node
        keys.push_back(kv); // Append to keys
        numKeys++;

        // Move keys to maintain order
        while (i >= 0 && kv < keys[i]) {
            keys[i + 1] = keys[i];
            i--;
        }
        keys[i + 1] = kv;

        // Write the updated node to disk
        tree->writeNode(*this);
    } else {
        // Find the child that will have the new key
        while (i >= 0 && kv < keys[i]) {
            i--;
        }
        i++;

        // Read the child node
        uint64_t childOffset = childOffsets[i];
        BTreeNode childNode = tree->readNode(childOffset);

        if (childNode.numKeys == 2 * degree - 1) {
            splitChild(i, degree, tree);
            if (kv > keys[i]) {
                i++;
            }
            childOffset = childOffsets[i];
            childNode = tree->readNode(childOffset);
        }
        childNode.insertNonFull(kv, degree, tree);
    }
}

void DiskBTree::BTreeNode::splitChild(int idx, int degree, DiskBTree* tree) {
    uint64_t childOffset = childOffsets[idx];
    BTreeNode childNode = tree->readNode(childOffset);

    // Create new node (sibling)
    uint64_t siblingOffset = tree->pageManager.allocatePage();
    BTreeNode siblingNode(childNode.isLeaf, siblingOffset);

    // Split keys and children
    siblingNode.numKeys = degree - 1;
    siblingNode.keys.assign(childNode.keys.begin() + degree, childNode.keys.end());
    childNode.keys.resize(degree - 1);
    childNode.numKeys = degree - 1;

    if (!childNode.isLeaf) {
        siblingNode.childOffsets.assign(childNode.childOffsets.begin() + degree, childNode.childOffsets.end());
        childNode.childOffsets.resize(degree);
    } else {
        siblingNode.nextLeafOffset = childNode.nextLeafOffset;
        childNode.nextLeafOffset = siblingOffset;
    }

    // Insert new key and child pointer to this node
    keys.insert(keys.begin() + idx, childNode.keys[degree - 1]);
    childOffsets.insert(childOffsets.begin() + idx + 1, siblingOffset);
    numKeys++;

    // Write nodes back to disk
    tree->writeNode(childNode);
    tree->writeNode(siblingNode);
    tree->writeNode(*this);
}

KeyValueWrapper* DiskBTree::search(const KeyValueWrapper& kv) {
    if (rootOffset != 0) {
        return readNode(rootOffset).search(kv, this);
    }
    return nullptr;
}

KeyValueWrapper* DiskBTree::BTreeNode::search(const KeyValueWrapper& kv, DiskBTree* tree) {
    int i = 0;

    while (i < numKeys && kv > keys[i]) {
        i++;
    }

    if (isLeaf) {
        if (i < numKeys && !(kv < keys[i]) && !(keys[i] < kv)) {
            // Found the key
            return &keys[i];
        }
        return nullptr;
    } else {
        // Traverse to child node
        uint64_t childOffset = childOffsets[i];
        BTreeNode childNode = tree->readNode(childOffset);
        return childNode.search(kv, tree);
    }
}

void DiskBTree::traverse() {
    if (rootOffset != 0) {
        readNode(rootOffset).traverse(this);
    }
}

void DiskBTree::BTreeNode::traverse(DiskBTree* tree) {
    if (isLeaf) {
        for (int i = 0; i < numKeys; i++) {
            keys[i].printKeyValue();
        }
        if (nextLeafOffset != 0) {
            BTreeNode nextLeaf = tree->readNode(nextLeafOffset);
            nextLeaf.traverse(tree);
        }
    } else {
        for (int i = 0; i < numKeys; i++) {
            // Traverse child node
            BTreeNode childNode = tree->readNode(childOffsets[i]);
            childNode.traverse(tree);
            // Optionally, print internal node keys
            // keys[i].printKeyValue();
        }
        // Traverse the last child
        BTreeNode childNode = tree->readNode(childOffsets[numKeys]);
        childNode.traverse(tree);
    }
}


void DiskBTree::scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result) {
    if (rootOffset != 0) {
        readNode(rootOffset).scan(startKey, endKey, this, result);
    }
}

void DiskBTree::BTreeNode::scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, DiskBTree* tree, std::vector<KeyValueWrapper>& result) {
    if (isLeaf) {
        for (const auto& kv : keys) {
            if (!(kv < startKey) && !(endKey < kv)) {
                result.push_back(kv);
            }
        }
        if (nextLeafOffset != 0 && keys.back() < endKey) {
            BTreeNode nextLeaf = tree->readNode(nextLeafOffset);
            nextLeaf.scan(startKey, endKey, tree, result);
        }
    } else {
        int i = 0;
        while (i < numKeys && keys[i] < startKey) {
            i++;
        }
        for (; i <= numKeys; i++) {
            if (i < childOffsets.size()) {
                BTreeNode childNode = tree->readNode(childOffsets[i]);
                childNode.scan(startKey, endKey, tree, result);
            }
            if (i < numKeys && keys[i] > endKey) {
                break;
            }
        }
    }
}


