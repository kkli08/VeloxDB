//
// Created by damian on 9/24/24.
//

//
// DiskBTree.cpp
//

#include "DiskBTree.h"
#include <stdexcept>
#include <algorithm>

// Constructor for building a new B+ tree from memtable data
DiskBTree::DiskBTree(const std::string& sstFileName, int degree, const std::vector<KeyValueWrapper>& keyValues)
    : degree(degree), pageManager(sstFileName), sstFileName(sstFileName) {
    // Check for valid degree (must be > 0)
    if (degree <= 0) {
        throw std::invalid_argument("B+ tree degree must be greater than zero.");
    }
    // Write placeholder metadata to offset 0
    Page metadataPage(Page::PageType::SST_METADATA);
    pageManager.writePage(0, metadataPage); // Reserve offset 0

    // Build the B+ tree from the sorted key-values
    auto rootNode = buildTree(keyValues);

    // Write the tree nodes to disk starting from the root
    rootNode->writeNode(this);

    // Set the root offset
    rootOffset = rootNode->selfOffset;

    // Update and write the metadata page with the actual root offset
    metadataPage.setMetadata(rootOffset, 0, 0, sstFileName);
    pageManager.writePage(0, metadataPage);
}



// Constructor for opening an existing SST file
DiskBTree::DiskBTree(const std::string& sstFileName, int degree)
    : degree(degree), pageManager(sstFileName), sstFileName(sstFileName) {
    // Check for valid degree (must be > 0)
    if (degree <= 0) {
        throw std::invalid_argument("B-tree degree must be greater than zero.");
    }
    // Read the root offset from metadata
    Page metadataPage = pageManager.readPage(0);
    uint64_t leafBegin, leafEnd;
    std::string fileName;
    metadataPage.getMetadata(rootOffset, leafBegin, leafEnd, fileName);
}

// Destructor
DiskBTree::~DiskBTree() {
    // Close the page manager
    pageManager.close();
}

// Build B+ tree from sorted key-values
std::shared_ptr<DiskBTree::BTreeNode> DiskBTree::buildTree(const std::vector<KeyValueWrapper>& keyValues) {
    // Create an empty root node
    auto rootNode = std::make_shared<BTreeNode>(true);

    for (const auto& kv : keyValues) {
        // // log
        // std::cout << "Inserting key: " << kv.kv.int_key() << std::endl;
        if (rootNode->keys.size() == 2 * degree - 1) {
            // Root is full, need to split
            auto newRoot = std::make_shared<BTreeNode>(false);
            newRoot->children.push_back(rootNode);
            newRoot->splitChild(0, degree);
            rootNode = newRoot;
        }
        rootNode->insertNonFull(kv, degree);
    }

    return rootNode;
}

// BTreeNode constructor
DiskBTree::BTreeNode::BTreeNode(bool isLeaf)
    : isLeaf(isLeaf), selfOffset(0) {}

// Insert non-full
void DiskBTree::BTreeNode::insertNonFull(const KeyValueWrapper& kv, int degree) {
    int i = keys.size() - 1;
    if (isLeaf) {
        keys.push_back(kv); // Add space for new key
        while (i >= 0 && kv < keys[i]) {
            keys[i + 1] = keys[i]; // Shift keys to make room
            --i;
        }
        keys[i + 1] = kv; // Insert new key
    } else {
        while (i >= 0 && kv < keys[i]) {
            --i;
        }
        ++i;
        // If the child is full, split it
        if (children[i]->keys.size() == (2 * degree - 1)) {
            splitChild(i, degree);
            if (kv >= keys[i]) {
                ++i;
            }
        }
        children[i]->insertNonFull(kv, degree);
    }

    // // Logging
    // if (isLeaf) {
    //     std::cout << "Inserted key " << kv.kv.int_key() << " into leaf node with keys: ";
    //     for (const auto& key : keys) {
    //         std::cout << key.kv.int_key() << " ";
    //     }
    //     std::cout << std::endl;
    // } else {
    //     std::cout << "Inserting key " << kv.kv.int_key() << " into internal node with keys: ";
    //     for (const auto& key : keys) {
    //         std::cout << key.kv.int_key() << " ";
    //     }
    //     std::cout << std::endl;
    // }
}




// Split child node
void DiskBTree::BTreeNode::splitChild(int idx, int degree) {
    auto y = children[idx];
    auto z = std::make_shared<BTreeNode>(y->isLeaf);

    int t = degree;

    if (y->isLeaf) {
        // Move the last t keys from y to z
        z->keys.assign(y->keys.begin() + t - 1, y->keys.end());
        y->keys.resize(t - 1);

        // Insert the first key of z into the parent
        keys.insert(keys.begin() + idx, z->keys[0]);

        // Since y is a leaf node, we don't remove the promoted key from z
    } else {
        // Move the last (t - 1) keys from y to z
        z->keys.assign(y->keys.begin() + t, y->keys.end());
        y->keys.resize(t);

        // Move the last t children from y to z
        z->children.assign(y->children.begin() + t, y->children.end());
        y->children.resize(t);

        // Promote the median key to the parent
        keys.insert(keys.begin() + idx, y->keys[t - 1]);

        // Remove the median key from y
        y->keys.resize(t - 1);
    }

    // Insert new child into this node
    children.insert(children.begin() + idx + 1, z);

    // // Logging
    // std::cout << "After splitting child at index " << idx << ":" << std::endl;
    // std::cout << "Parent node keys: ";
    // for (const auto& key : keys) {
    //     std::cout << key.kv.int_key() << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "Left child keys: ";
    // for (const auto& key : y->keys) {
    //     std::cout << key.kv.int_key() << " ";
    // }
    // std::cout << std::endl;
    // std::cout << "Right child keys: ";
    // for (const auto& key : z->keys) {
    //     std::cout << key.kv.int_key() << " ";
    // }
    // std::cout << std::endl;
}






// Write node and its children to disk
void DiskBTree::BTreeNode::writeNode(DiskBTree* tree) {
    if (selfOffset == 0) {
        selfOffset = tree->pageManager.allocatePage();
        // // logging
        // std::cout << "Writing node at offset: " << selfOffset << std::endl;
    }

    Page page(isLeaf ? Page::PageType::LEAF_NODE : Page::PageType::INTERNAL_NODE);

    if (isLeaf) {
        for (const auto& kv : keys) {
            page.addLeafEntry(kv);
        }

        // // log
        // std::cout << "Leaf node keys at offset " << selfOffset << ": ";
        // for (const auto& kv : keys) {
        //     std::cout << kv.kv.int_key() << " ";
        // }
        // std::cout << std::endl;
    } else {
        std::vector<uint64_t> childOffsets;
        for (auto& child : children) {
            child->writeNode(tree);
            childOffsets.push_back(child->selfOffset);
        }
        for (const auto& offset : childOffsets) {
            page.addChildOffset(offset);
        }
        for (const auto& key : keys) {
            page.addKey(key);
        }
        // // log
        // std::cout << "Internal node keys at offset " << selfOffset << ": ";
        // for (const auto& key : keys) {
        //     std::cout << key.kv.int_key() << " ";
        // }
        // std::cout << std::endl;
        // std::cout << "Child offsets: ";
        // for (const auto& offset : childOffsets) {
        //     std::cout << offset << " ";
        // }
        // std::cout << std::endl;
    }

    tree->pageManager.writePage(selfOffset, page);
}


// Set the offset after writing to disk
void DiskBTree::BTreeNode::setOffset(uint64_t offset) {
    selfOffset = offset;
}

// Search for a key in the B+ tree
KeyValueWrapper* DiskBTree::search(const KeyValueWrapper& kv) {
    return searchInNode(rootOffset, kv);
}

// Search for a key starting from a node
KeyValueWrapper* DiskBTree::searchInNode(uint64_t nodeOffset, const KeyValueWrapper& kv) {
    Page page = pageManager.readPage(nodeOffset);
    Page::PageType pageType = page.getPageType();

    if (pageType == Page::PageType::LEAF_NODE) {
        const auto& entries = page.getLeafEntries();
        auto it = std::lower_bound(entries.begin(), entries.end(), kv);
        if (it != entries.end() && *it == kv) {
            return new KeyValueWrapper(*it);
        }
        return nullptr;
    } else if (pageType == Page::PageType::INTERNAL_NODE) {
        const auto& keys = page.getInternalKeys();
        const auto& childOffsets = page.getChildOffsets();

        int i = 0;
        while (i < keys.size() && kv >= keys[i]) {
            ++i;
        }

        return searchInNode(childOffsets[i], kv);
    } else {
        throw std::runtime_error("Invalid page type during search");
    }
}



// Scan keys within a range
void DiskBTree::scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result) {
    scanInNode(rootOffset, startKey, endKey, result);
}

// Scan keys starting from a node
void DiskBTree::scanInNode(uint64_t nodeOffset, const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result) {
    Page page = pageManager.readPage(nodeOffset);
    if (page.getPageType() == Page::PageType::LEAF_NODE) {
        const auto& entries = page.getLeafEntries();
        for (const auto& kv : entries) {
            if (kv >= startKey && kv <= endKey) {
                result.push_back(kv);
            } else if (kv > endKey) {
                break;
            }
        }
    } else if (page.getPageType() == Page::PageType::INTERNAL_NODE) {
        const auto& keys = page.getInternalKeys();
        const auto& childOffsets = page.getChildOffsets();
        int i = 0;
        while (i < keys.size() && startKey > keys[i]) {
            ++i;
        }
        for (; i < childOffsets.size(); ++i) {
            scanInNode(childOffsets[i], startKey, endKey, result);
            if (i < keys.size() && keys[i] > endKey) {
                break;
            }
        }
    } else {
        throw std::runtime_error("Invalid page type during scan");
    }
}

// Get the SST file name
std::string DiskBTree::getFileName() const {
    return sstFileName;
}

void DiskBTree::setBufferPoolParameters(size_t capacity, EvictionPolicy policy) {
    pageManager.setBufferPoolParameters(capacity, policy);
}



