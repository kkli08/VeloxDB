// DiskBTree.cpp

#include "DiskBTree.h"
#include "Page.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>

DiskBTree::DiskBTree(const std::string& sstFileName, const std::vector<KeyValueWrapper>& keyValues, size_t pageSize)
    : sstFileName(sstFileName), pageSize(pageSize), root(nullptr)
{
    // Constructor for creating a new SST file
    totalKeyValueCount = keyValues.size();
    pageManager = std::make_shared<PageManager>(sstFileName, pageSize);
    // Step 1: Write placeholder metadata to offset 0
    Page metadataPage(Page::PageType::SST_METADATA);
    pageManager->writePage(0, metadataPage); // Reserve offset 0

    // Step 2: Split keyValues into leaf pages
    splitInputPairs(keyValues);

    // Step 3: Compute degree and height
    computeDegreeAndHeight();

    // Step 4: Build the tree
    buildTree();

    // Step 5: Write the tree into the SST file
    writeTreeToSST();

    // Step 6: Set the root offset
    rootOffset = root->offset;

    // Step 7: Update and write the metadata page with the actual root offset
    metadataPage.setMetadata(rootOffset, leafBeginOffset, leafEndOffset, sstFileName);
    pageManager->writePage(0, metadataPage);

    // After writing, clear the in-memory structures to free memory
    for (auto node : allNodes) {
        delete node;
    }
    allNodes.clear();
    root = nullptr;
    leafPages.clear();
    leafPageSmallestKeys.clear();
    levels.clear();
}

DiskBTree::DiskBTree(const std::string& sstFileName)
    : sstFileName(sstFileName), root(nullptr)
{
    // Constructor for reading an existing SST file
    pageManager = std::make_shared<PageManager>(sstFileName);
    // Read the metadata page from offset 0
    Page metadataPage = pageManager->readPage(0);

    // Extract metadata
    std::string fileName;
    metadataPage.getMetadata(rootOffset, leafBeginOffset, leafEndOffset, fileName);

    // sstFileName is already set; ensure it matches the metadata (optional)
    if (sstFileName != fileName) {
        fileName = sstFileName;
        // std::cerr << "Warning: SST file name does not match metadata file name." << std::endl;
    }

    // Since the tree is static, we don't load any nodes into memory
    // We rely on reading pages from disk during search and scan operations
}

DiskBTree::DiskBTree(const std::string& sstFileName, const std::string& leafsFileName, const std::vector<KeyValueWrapper>& leafPageSmallestKeys, int numOfPages, int totalKvs)
    : sstFileName(sstFileName), root(nullptr), leafPageSmallestKeys(leafPageSmallestKeys)
{
    // Constructor for creating a new SST file from existing leaf pages
    totalKeyValueCount = totalKvs;
    int actual_KV_read = 0;
    pageManager = std::make_shared<PageManager>(sstFileName);
    // cout << "DiskBTree::DiskBTree() Leaf file name: " << leafsFileName << endl;
    // Step 1: Write placeholder metadata to offset 0
    Page metadataPage(Page::PageType::SST_METADATA);
    pageManager->writePage(0, metadataPage); // Reserve offset 0

    // Step 2: Copy the leaf pages from the .leafs file into the SST file
    // Initialize variables
    uint64_t currentOffset = pageSize; // Start after metadata page
    std::vector<uint64_t> leafPageOffsets; // Offsets of leaf pages in the SST file

    PageManager leafPageManager(leafsFileName);
    // cout << "DiskBTree::DiskBTree(): Number of Pages to read: " << numOfPages << std::endl;


    for(int i = 0; i < leafPageSmallestKeys.size(); i++) {
        // cout << "DiskBTree::DiskBTree() read page offset: " << currentOffset << endl;
        uint64_t offset = currentOffset;
        Page leafPage = leafPageManager.readPage(currentOffset);
        actual_KV_read += leafPage.getLeafEntries().size();

        // Set the nextLeafOffset of the previous leaf page

        if (i > 0) {
            Page PreviousPage = pageManager->readPage(offset-pageSize);
            PreviousPage.setNextLeafOffset(offset);
            // Re-write the previous leaf page to update the nextLeafOffset
            pageManager->writePage(leafPageOffsets[i-1], PreviousPage);
        }

        // leafPage.printType();
        pageManager->writePage(currentOffset, leafPage);
        leafPageOffsets.push_back(currentOffset);
        currentOffset += pageSize;
    }
    // cout << "DiskBTree::DiskBTree(): actual_KV_read == " << actual_KV_read << endl;


    // Set leafBeginOffset and leafEndOffset
    if (!leafPageOffsets.empty()) {
        // cout << "DiskBTree::DiskBTree() Leaf Page offsets not empty" << endl;
        leafBeginOffset = leafPageOffsets.front();
        leafEndOffset = leafPageOffsets.back();
    } else {
        leafBeginOffset = 0;
        leafEndOffset = 0;
    }

    // Step 2.5: Compute degree and height
    computeDegreeAndHeightFromLeafKeys(leafPageSmallestKeys);

    // Step 3: Build the tree
    // cout << "DiskBTree::DiskBTree(): vec size of smallest key: " << leafPageSmallestKeys.size() << endl;
    // cout << "DiskBTree::DiskBTree(): vec size of leafPageOffsets: " << leafPageOffsets.size() << endl;
    buildTreeFromLeafPageKeys(leafPageSmallestKeys, leafPageOffsets);

    // Step 4: Write the internal nodes into the SST file
    writeTreeToSSTWithLeafOffsets(leafPageOffsets);

    // Step 5: Set the root offset
    rootOffset = root->offset;

    // Step 6: Update and write the metadata page with the actual root offset
    metadataPage.setMetadata(rootOffset, leafBeginOffset, leafEndOffset, sstFileName);
    pageManager->writePage(0, metadataPage);

    // After writing, clear the in-memory structures to free memory
    for (auto node : allNodes) {
        delete node;
    }
    allNodes.clear();
    root = nullptr;
    levels.clear();
}

DiskBTree::~DiskBTree()
{
    // Delete all allocated nodes
    for (auto node : allNodes) {
        delete node;
    }
}

std::string DiskBTree::getFileName() const {
    return sstFileName;
}

void DiskBTree::setBufferPoolParameters(size_t capacity, EvictionPolicy policy) {
    pageManager->setBufferPoolParameters(capacity, policy);
}

long long DiskBTree::getCacheHit() const {
    return pageManager->getCacheHit();
}

KeyValueWrapper* DiskBTree::search(const KeyValueWrapper& kv) {
    // std::cout << "DiskBTree::search() --> bp 0" << std::endl;
    // Start from the root offset
    uint64_t currentOffset = rootOffset;

    int i = 1;
    while (true) {
        // std::cout << "DiskBTree::search() --> bp " << i++ << std::endl;

        // Read the page from disk
        Page currentPage = pageManager->readPage(currentOffset);

        if (currentPage.getPageType() == Page::PageType::INTERNAL_NODE) {
            // std::cout << "DiskBTree::search() --> INTERNAL_NODE" << std::endl;

            // Internal node
            const std::vector<KeyValueWrapper>& keys = currentPage.getInternalKeys();
            const std::vector<uint64_t>& childOffsets = currentPage.getChildOffsets();

            // Find the child to follow
            size_t i = 0;
            while (i < keys.size() && kv > keys[i]) {
                i++;
            }
            // Now, i is the index of the child to follow
            currentOffset = childOffsets[i];

        } else if (currentPage.getPageType() == Page::PageType::LEAF_NODE) {
            // std::cout << "DiskBTree::search() --> LEAF_NODE" << std::endl;
            // Leaf node
            // Optionally, check Bloom filter first
            if (currentPage.leafBloomFilterContains(kv)) {
                // Bloom filter indicates the key may be present
                const std::vector<KeyValueWrapper>& kvPairs = currentPage.getLeafEntries();

                // Perform binary search on kvPairs
                auto it = std::lower_bound(kvPairs.begin(), kvPairs.end(), kv);

                if (it != kvPairs.end() && *it == kv) {
                    // Key found
                    // Return a copy of the found key-value pair
                    return new KeyValueWrapper(*it);
                }

            }
            // std::cout << "DiskBTree::search() --> LEAF_NODE BP end" << std::endl;
            // Key not found
            return nullptr;
        } else {
            // Invalid page type
            std::cerr << "Invalid page type encountered during search." << std::endl;
            return nullptr;
        }
    }
}

void DiskBTree::scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result) {
    // Start from the root offset
    uint64_t currentOffset = rootOffset;

    // Traverse the tree to find the starting leaf node
    while (true) {
        // Read the page from disk
        Page currentPage = pageManager->readPage(currentOffset);

        if (currentPage.getPageType() == Page::PageType::INTERNAL_NODE) {
            // Internal node
            const std::vector<KeyValueWrapper>& keys = currentPage.getInternalKeys();
            const std::vector<uint64_t>& childOffsets = currentPage.getChildOffsets();

            // Find the child to follow
            size_t i = 0;
            while (i < keys.size() && startKey > keys[i]) {
                i++;
            }
            // Now, i is the index of the child to follow
            currentOffset = childOffsets[i];

        } else if (currentPage.getPageType() == Page::PageType::LEAF_NODE) {
            // We have reached the leaf node where startKey would be
            break;

        } else {
            // Invalid page type
            std::cerr << "Invalid page type encountered during scan." << std::endl;
            return;
        }
    }

    // Now, currentOffset points to the leaf node where startKey would be
    bool done = false;

    while (!done) {
        // Read the leaf page from disk
        Page currentPage = pageManager->readPage(currentOffset);

        // Process current leaf page
        const std::vector<KeyValueWrapper>& kvPairs = currentPage.getLeafEntries();

        // Iterate over the kvPairs
        for (const auto& kv : kvPairs) {
            if (kv < startKey) {
                // Skip keys less than startKey
                continue;
            }
            if (kv > endKey) {
                // Reached keys beyond endKey
                done = true;
                break;
            }
            // Key is within [startKey, endKey], add to result
            result.push_back(kv);
        }

        if (done) {
            break;
        }

        // Move to the next leaf page
        uint64_t nextLeafOffset = currentPage.getNextLeafOffset();
        if (nextLeafOffset == 0) {
            // No more leaf pages
            break;
        }

        currentOffset = nextLeafOffset;
    }
}

void DiskBTree::splitInputPairs(const std::vector<KeyValueWrapper>& keyValues) {
    size_t currentIndex = 0;
    size_t totalKeys = keyValues.size();

    while (currentIndex < totalKeys) {
        Page leafPage(Page::PageType::LEAF_NODE);

        // Build a bloom filter for the leaf page
        size_t m = 1024; // Number of bits in bloom filter, can be adjusted
        size_t n = 100;  // Expected number of elements, can be adjusted
        leafPage.buildLeafBloomFilter(m, n);

        size_t estimatedPageSize = leafPage.getBaseSize(); // Get base size of the page

        while (currentIndex < totalKeys) {
            const KeyValueWrapper& kv = keyValues[currentIndex];

            // Estimate the size of the kv pair
            size_t kvSize = kv.getSerializedSize();

            if (estimatedPageSize + kvSize > pageSize) {
                // Page size limit reached
                break;
            }

            // Add kv to leaf page
            leafPage.addLeafEntry(kv);

            // Add kv to bloom filter
            leafPage.addToLeafBloomFilter(kv);

            estimatedPageSize += kvSize;
            currentIndex++;
        }

        // Add the leaf page to the vector of leaf pages
        leafPages.push_back(leafPage);

        // Record the smallest key in this leaf page
        if (!leafPage.getLeafEntries().empty()) {
            leafPageSmallestKeys.push_back(leafPage.getLeafEntries().front());
        }
    }
}

void DiskBTree::computeDegreeAndHeight() {
    // Compute the maximum number of keys and child offsets that can fit into an internal node page

    size_t pageOverhead = sizeof(Page::PageType) + sizeof(uint16_t); // pageType and numEntries

    // Estimate size of a key
    size_t keySize = 0;
    if (!leafPageSmallestKeys.empty()) {
        keySize = leafPageSmallestKeys[0].getSerializedSize();
    } else {
        keySize = sizeof(KeyValueWrapper); // Fallback estimate
    }

    size_t childOffsetSize = sizeof(uint64_t);

    size_t numerator = pageSize - pageOverhead + keySize;
    size_t denominator = childOffsetSize + keySize;

    degree = numerator / denominator;

    if (degree < 2) {
        degree = 2;
    }

    size_t totalNumLeafNodes = leafPages.size();

    double heightEstimate = std::log(static_cast<double>(totalNumLeafNodes)) / std::log(static_cast<double>(degree));

    height = static_cast<size_t>(std::ceil(heightEstimate));

    if (height < 1) {
        height = 1;
    }
}

void DiskBTree::computeDegreeAndHeightFromLeafKeys(const std::vector<KeyValueWrapper>& leafPageSmallestKeys) {
    // Compute the maximum number of keys and child offsets that can fit into an internal node page

    size_t pageOverhead = sizeof(Page::PageType) + sizeof(uint16_t); // pageType and numEntries

    // Estimate size of a key
    size_t keySize = 0;
    if (!leafPageSmallestKeys.empty()) {
        // Serialize the first key to get its size
        // std::string keyData;
        // leafPageSmallestKeys[0].kv.SerializeToString(&keyData);
        // keySize = keyData.size();
        keySize = leafPageSmallestKeys[0].getSerializedSize();

    } else {
        keySize = sizeof(KeyValueWrapper); // Fallback estimate
    }

    size_t childOffsetSize = sizeof(uint64_t);

    // Compute the maximum number of keys that can fit in a page
    // totalSize = pageOverhead + numKeys * keySize + (numKeys + 1) * childOffsetSize <= pageSize
    // Solve for numKeys:
    // numKeys * (keySize + childOffsetSize) <= pageSize - pageOverhead - childOffsetSize
    // numKeys <= (pageSize - pageOverhead - childOffsetSize) / (keySize + childOffsetSize)

    size_t numerator = pageSize - pageOverhead - childOffsetSize;
    size_t denominator = keySize + childOffsetSize;

    degree = numerator / denominator;

    if (degree < 2) {
        // Minimum degree should be at least 2
        degree = 2;
    }

    // Now, compute the height of the tree

    // Total number of leaf nodes
    size_t totalNumLeafNodes = leafPageSmallestKeys.size();

    if (totalNumLeafNodes == 0) {
        height = 1; // At least height 1
    } else {
        double heightEstimate = std::log(static_cast<double>(totalNumLeafNodes)) / std::log(static_cast<double>(degree));
        height = static_cast<size_t>(std::ceil(heightEstimate));

        // Ensure height is at least 1
        if (height < 1) {
            height = 1;
        }
    }
}

void DiskBTree::buildTree() {
    // Build the tree in memory using BTreeNodes
    // Start from the leaf level

    std::vector<BTreeNode*> currentLevel;

    size_t numLeafPages = leafPages.size();
    size_t index = 0;

    while (index < numLeafPages) {
        BTreeNode* node = new BTreeNode(false); // Not a leaf node
        allNodes.push_back(node);

        // Add children (indices to leaf pages)
        size_t end = std::min(index + degree, numLeafPages);

        for (size_t i = index; i < end; ++i) {
            if (i > index) {
                node->keys.push_back(leafPageSmallestKeys[i]);
            }
            node->leafPageIndices.push_back(i);
        }

        currentLevel.push_back(node);
        index = end;
    }

    // Add this level to levels
    levels.push_back(currentLevel);

    // Now, build higher levels
    while (currentLevel.size() > 1) {
        std::vector<BTreeNode*> nextLevel;
        index = 0;
        size_t numNodes = currentLevel.size();

        while (index < numNodes) {
            BTreeNode* node = new BTreeNode(false);
            allNodes.push_back(node);

            size_t end = std::min(index + degree, numNodes);

            for (size_t i = index; i < end; ++i) {
                if (i > index) {
                    node->keys.push_back(currentLevel[i]->keys.front());
                }
                node->children.push_back(currentLevel[i]);
            }

            nextLevel.push_back(node);
            index = end;
        }

        levels.push_back(nextLevel);
        currentLevel = nextLevel;
    }

    // The last level contains the root
    root = levels.back().front();
}

void DiskBTree::buildTreeFromLeafPageKeys(const std::vector<KeyValueWrapper>& leafPageSmallestKeys, const std::vector<uint64_t>& leafPageOffsets) {
    // Build the tree in memory using BTreeNodes
    // Start from the leaf level

    std::vector<BTreeNode*> currentLevel;

    size_t numLeafPages = leafPageOffsets.size();
    size_t index = 0;

    // Build the first level (nodes pointing to leaf pages)
    while (index < numLeafPages) {
        BTreeNode* node = new BTreeNode(false); // Not a leaf node
        allNodes.push_back(node);

        // Add children (indices to leaf pages)
        size_t end = std::min(index + degree, numLeafPages);

        for (size_t i = index; i < end; ++i) {
            if (i > index) {
                node->keys.push_back(leafPageSmallestKeys[i]);
            }
            node->leafPageIndices.push_back(i); // Store the index of the leaf page
        }

        currentLevel.push_back(node);
        index = end;
    }

    // Add this level to levels
    levels.push_back(currentLevel);

    // Now, build higher levels
    while (currentLevel.size() > 1) {
        std::vector<BTreeNode*> nextLevel;
        index = 0;
        size_t numNodes = currentLevel.size();

        while (index < numNodes) {
            BTreeNode* node = new BTreeNode(false);
            allNodes.push_back(node);

            size_t end = std::min(index + degree, numNodes);

            for (size_t i = index; i < end; ++i) {
                if (i > index) {
                    node->keys.push_back(currentLevel[i]->keys.front());
                }
                node->children.push_back(currentLevel[i]);
            }

            nextLevel.push_back(node);
            index = end;
        }

        levels.push_back(nextLevel);
        currentLevel = nextLevel;
    }

    // The last level contains the root
    root = levels.back().front();
}

void DiskBTree::writeTreeToSST() {
    uint64_t currentOffset = pageSize; // Start after the metadata page

    // 1. Leaf Pages
    std::vector<uint64_t> leafPageOffsets;
    for (size_t i = 0; i < leafPages.size(); ++i) {
        uint64_t offset = currentOffset;

        // Set the nextLeafOffset of the previous leaf page
        if (i > 0) {
            leafPages[i - 1].setNextLeafOffset(offset);
            // Re-write the previous leaf page to update the nextLeafOffset
            pageManager->writePage(leafPageOffsets[i - 1], leafPages[i - 1]);
        }

        // Write the current leaf page
        pageManager->writePage(offset, leafPages[i]);
        leafPageOffsets.push_back(offset);
        currentOffset += pageSize;
    }

    // Ensure the last leaf page's nextLeafOffset is set to 0
    if (!leafPages.empty()) {
        leafPages.back().setNextLeafOffset(0);
        // Re-write the last leaf page to update the nextLeafOffset
        pageManager->writePage(leafPageOffsets.back(), leafPages.back());
    }

    // Set leafBeginOffset and leafEndOffset
    if (!leafPageOffsets.empty()) {
        leafBeginOffset = leafPageOffsets.front();
        leafEndOffset = leafPageOffsets.back();
    } else {
        leafBeginOffset = 0;
        leafEndOffset = 0;
    }

    // 2. Internal Nodes
    // Process levels from the lowest to the highest
    for (size_t levelIndex = 0; levelIndex < levels.size(); ++levelIndex) {
        std::vector<BTreeNode*>& nodes = levels[levelIndex];
        for (BTreeNode* node : nodes) {
            // Assign currentOffset to node's offset
            node->offset = currentOffset;

            // Create a Page for this node
            Page internalPage(Page::PageType::INTERNAL_NODE);

            // Add keys to the page
            for (const auto& key : node->keys) {
                internalPage.addKey(key);
            }

            // Add child offsets
            if (levelIndex == 0) {
                // This level points to leaf pages
                for (size_t leafIndex : node->leafPageIndices) {
                    uint64_t childOffset = leafPageOffsets[leafIndex];
                    internalPage.addChildOffset(childOffset);
                }
            } else {
                // This level points to internal nodes
                for (BTreeNode* childNode : node->children) {
                    internalPage.addChildOffset(childNode->offset);
                }
            }

            // Write the page
            pageManager->writePage(node->offset, internalPage);
            currentOffset += pageSize;
        }
    }

    // The root node should be the last node processed
    rootOffset = root->offset;
}

void DiskBTree::writeTreeToSSTWithLeafOffsets(const std::vector<uint64_t>& leafPageOffsets) {
    uint64_t currentOffset = leafEndOffset + pageSize; // Start after the leaf pages

    // 2. Internal Nodes
    // Process levels from the lowest to the highest
    for (size_t levelIndex = 0; levelIndex < levels.size(); ++levelIndex) {
        std::vector<BTreeNode*>& nodes = levels[levelIndex];
        for (BTreeNode* node : nodes) {
            // Assign currentOffset to node's offset
            node->offset = currentOffset;

            // Create a Page for this node
            Page internalPage(Page::PageType::INTERNAL_NODE);

            // Add keys to the page
            for (const auto& key : node->keys) {
                internalPage.addKey(key);
            }

            // Add child offsets
            if (levelIndex == 0) {
                // This level points to leaf pages
                for (size_t leafIndex : node->leafPageIndices) {
                    uint64_t childOffset = leafPageOffsets[leafIndex];
                    internalPage.addChildOffset(childOffset);
                }
            } else {
                // This level points to internal nodes
                for (BTreeNode* childNode : node->children) {
                    internalPage.addChildOffset(childNode->offset);
                }
            }

            // Write the page
            pageManager->writePage(node->offset, internalPage);
            currentOffset += pageSize;
        }
    }

    // The root node should already have its offset set
    // No further action needed
}

void DiskBTree::printKVs() const {
    uint64_t currentOffset = getLeafBeginOffset();
    bool done = false;

    while (!done) {
        // Read the leaf page from disk
        Page currentPage = pageManager->readPage(currentOffset);

        // Process current leaf page
        const std::vector<KeyValueWrapper>& kvPairs = currentPage.getLeafEntries();

        // Iterate over the kvPairs
        for (const auto& kv : kvPairs) {
            // for testing purpose, we only print int value
            cout << "Key = " << kv.kv.int_key() << " Value = " << kv.kv.int_value() << endl;
        }

        if (done) {
            break;
        }

        // Move to the next leaf page
        uint64_t nextLeafOffset = currentPage.getNextLeafOffset();
        if (nextLeafOffset == 0) {
            // No more leaf pages
            break;
        }

        currentOffset = nextLeafOffset;
    }
}