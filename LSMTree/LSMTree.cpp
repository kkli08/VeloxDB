// LSMTree.cpp


#include "LSMTree.h"
#include <iostream>
#include <stdexcept>
#include <queue>

// Constructor
LSMTree::LSMTree(size_t memtableSize, const std::string& dbPath)
    : dbPath(dbPath), lsmFilePath(dbPath + "/manifest.lsm") {
    // Create the database directory if it doesn't exist
    if (!fs::exists(this->dbPath)) {
        fs::create_directories(this->dbPath);
    }

    // std::ifstream ifs(lsmFilePath, std::ios::binary);
    // if (!ifs.is_open()) {
    //     throw std::runtime_error("LSMTree::LSMTree() Failed to open LSM tree file for reading");
    // }

    // Initialize memtable
    memtable = std::make_unique<Memtable>(static_cast<int>(memtableSize));

    // Initialize level capacities
    // Level 1 capacity is same as memtable size
    levelMaxSizes.push_back(memtableSize); // Level 1

    // Initialize LSM tree
    initializeLSM();
}

// Destructor
LSMTree::~LSMTree() {
    // Save the state to the .lsm file upon destruction
    try {
        saveState();
    } catch (const std::exception& e) {
        std::cerr << "Error saving LSM tree state: " << e.what() << std::endl;
    }
}

// Initialize LSM tree by loading existing state or setting up a new one
void LSMTree::initializeLSM() {
    if (fs::exists(lsmFilePath)) {
        // If manifest file exists, load the state
        loadState();
    } else {
        // No existing LSM file, initialize empty levels
        levels.clear();
    }
}

// Save the state of the LSM tree to a .lsm file
void LSMTree::saveState() {
    // Output the manifest file path for debugging
    std::cout << "manifest file path: " << lsmFilePath << std::endl;

    // Ensure the manifest file exists; create it if it doesn't
    if (!std::filesystem::exists(lsmFilePath)) {
        std::ofstream newFile(lsmFilePath, std::ios::binary); // This creates the file
        // if (!newFile) {
        //     throw std::runtime_error("LSMTree::saveState() Failed to create manifest file");
        // }
        newFile.close();
    }

    // Open the manifest file for writing
    std::ofstream ofs(lsmFilePath, std::ios::binary | std::ios::trunc);
    // if (!ofs) {
    //     throw std::runtime_error("LSMTree::saveState() Failed to open LSM tree file for writing");
    // }

    // Write the number of levels (excluding memtable)
    size_t numLevels = levels.size();
    ofs.write(reinterpret_cast<const char*>(&numLevels), sizeof(numLevels));

    // Write the SSTable information for each level
    for (size_t i = 0; i < numLevels; ++i) {
        // Write the level number
        int levelNumber = static_cast<int>(i + 1); // Levels start from 1
        ofs.write(reinterpret_cast<const char*>(&levelNumber), sizeof(levelNumber));

        // Write the SSTable file name (only the filename, not the full path)
        if (levels[i]) {
            // Extract only the filename from the full path
            std::filesystem::path fullPath = levels[i]->getFileName();
            std::string sstableFileName = fullPath.filename().string();
            size_t fileNameLength = sstableFileName.size();

            ofs.write(reinterpret_cast<const char*>(&fileNameLength), sizeof(fileNameLength));
            ofs.write(sstableFileName.c_str(), fileNameLength);
        } else {
            // No SSTable at this level
            size_t fileNameLength = 0;
            ofs.write(reinterpret_cast<const char*>(&fileNameLength), sizeof(fileNameLength));
        }

        // Write the level capacity
        size_t levelCapacity = levelMaxSizes[i];
        ofs.write(reinterpret_cast<const char*>(&levelCapacity), sizeof(levelCapacity));
    }

    ofs.close();
}

// Load the state of the LSM tree from a .lsm file
void LSMTree::loadState() {
    std::ifstream ifs(lsmFilePath, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("LSMTree::loadState() Failed to open LSM tree file for reading");
    }

    // Read the number of levels (excluding memtable)
    size_t numLevels;
    ifs.read(reinterpret_cast<char*>(&numLevels), sizeof(numLevels));

    levels.resize(numLevels);
    levelMaxSizes.resize(numLevels);

    // Read the SSTable information for each level
    for (size_t i = 0; i < numLevels; ++i) {
        // Read the level number
        int levelNumber;
        ifs.read(reinterpret_cast<char*>(&levelNumber), sizeof(levelNumber));

        // Read the SSTable file name
        size_t fileNameLength;
        ifs.read(reinterpret_cast<char*>(&fileNameLength), sizeof(fileNameLength));

        if (fileNameLength > 0) {
            std::string sstableFileName(fileNameLength, '\0');
            ifs.read(&sstableFileName[0], fileNameLength);

            // Construct full path to the SSTable file
            fs::path sstablePath = dbPath / sstableFileName;

            // Verify that the SSTable file exists
            if (!fs::exists(sstablePath)) {
                throw std::runtime_error("LSMTree::loadState() SSTable file does not exist: " + sstablePath.string());
            }

            // Create a new DiskBTree instance with the SSTable file
            levels[i] = std::make_shared<DiskBTree>(sstablePath.string());
        } else {
            // No SSTable at this level
            levels[i] = nullptr;
        }

        // Read the level capacity
        size_t levelCapacity;
        ifs.read(reinterpret_cast<char*>(&levelCapacity), sizeof(levelCapacity));
        levelMaxSizes[i] = levelCapacity;
    }

    ifs.close();
}


// Get the number of levels in the LSM tree (including memtable)
size_t LSMTree::getNumLevels() const {
    // levels.size() gives the number of levels excluding memtable (Level 0)
    return levels.size() + 1; // +1 for memtable
}

// Set the database path
void LSMTree::setDBPath(const std::string& path) {
    dbPath = path;
    lsmFilePath = dbPath / "manifest.lsm";
}

// Get the database path
std::string LSMTree::getDBPath() const {
    return dbPath.string();
}

// Insert a key-value pair into the LSM tree
void LSMTree::put(const KeyValueWrapper& kv) {
    // Insert into the memtable
    memtable->put(kv);

    // Check if the memtable needs to be flushed
    if (memtable->getCurrentSize() == memtable->getThreshold()) {
        flushMemtableToLevel1();
    }
}

// Search for a key-value pair in the LSM tree
KeyValueWrapper LSMTree::get(const KeyValueWrapper& kv) {
    // First, search in the memtable
    KeyValueWrapper result = memtable->get(kv);
    if (!result.isEmpty()) {
        if (!result.isTombstone()) {
            // Found and not deleted
            return result;
        } else {
            // Key is deleted
            return KeyValueWrapper(); // Return default (not found)
        }
    }
    // std::cout << "LSMTree::get(), Not found in memtable, now try to find in disk" << std::endl;
    // Not found in memtable, search in SSTables from Level 1 upwards
    for (size_t levelIndex = 0; levelIndex < levels.size(); ++levelIndex) {
        // std::cout << "LSMTree::get(), levelIndex = " << levelIndex << std::endl;
        std::shared_ptr<DiskBTree> sst = levels[levelIndex];
        if (sst) {
            KeyValueWrapper* kvPtr = sst->search(kv);
            if (kvPtr && !kvPtr->isEmpty()) {
                if (!kvPtr->isTombstone()) {
                    // Found and not deleted
                    return *kvPtr;
                } else {
                    // Key is deleted
                    return KeyValueWrapper(); // Return default (not found)
                }
            }
        }
    }

    // Not found in any level
    return KeyValueWrapper(); // Return default (empty) KeyValueWrapper
}

// In LSMTree.cpp

void LSMTree::scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result) {
    // Temporary storage for results from each level
    std::vector<std::vector<KeyValueWrapper>> levelResults;

    // Scan the memtable
    std::vector<KeyValueWrapper> memtableResults;
    std::set<KeyValueWrapper> memtableKeys;
    memtable->scan(startKey, endKey, memtableKeys);
    for (auto& kv : memtableKeys) {
        memtableResults.push_back(kv);
    }
    levelResults.push_back(memtableResults);

    // For each level from Level 1 upwards
    for (size_t levelIndex = 0; levelIndex < levels.size(); ++levelIndex) {
        std::shared_ptr<DiskBTree> sst = levels[levelIndex];
        if (sst) {
            std::vector<KeyValueWrapper> sstResults;
            sst->scan(startKey, endKey, sstResults);
            levelResults.push_back(sstResults);
        }
    }

    // Initialize a min-heap to merge the sorted sequences
    struct HeapNode {
        KeyValueWrapper kv;
        size_t levelIndex;
        size_t vectorIndex;

        // Define comparison operator for min-heap
        bool operator>(const HeapNode& other) const {
            return kv > other.kv;
        }
    };

    std::priority_queue<HeapNode, std::vector<HeapNode>, std::greater<HeapNode>> minHeap;

    // Initialize indices for each level
    std::vector<size_t> indices(levelResults.size(), 0);

    // Push the first element from each level into the heap
    for (size_t i = 0; i < levelResults.size(); ++i) {
        if (!levelResults[i].empty()) {
            HeapNode node{levelResults[i][0], i, 0};
            minHeap.push(node);
        }
    }

    KeyValueWrapper currentKey;
    KeyValueWrapper currentKV;
    bool hasCurrentKV = false;

    while (!minHeap.empty()) {
        HeapNode node = minHeap.top();
        minHeap.pop();

        KeyValueWrapper key = node.kv;

        if (!hasCurrentKV || key != currentKey) {
            // New key encountered
            if (hasCurrentKV && !currentKV.isTombstone()) {
                // Add the previous key-value to the result
                result.push_back(currentKV);
            }
            currentKey = key;
            currentKV = node.kv;
            hasCurrentKV = true;
        } else {
            // Same key, keep the one with the highest sequence number
            if (node.kv.sequenceNumber > currentKV.sequenceNumber) {
                currentKV = node.kv;
            }
        }

        // Move to the next element in the same level
        indices[node.levelIndex]++;
        if (indices[node.levelIndex] < levelResults[node.levelIndex].size()) {
            HeapNode nextNode{
                levelResults[node.levelIndex][indices[node.levelIndex]],
                node.levelIndex,
                indices[node.levelIndex]
            };
            minHeap.push(nextNode);
        }
    }

    // After processing all nodes, check if there's a remaining key
    if (hasCurrentKV && !currentKV.isTombstone()) {
        result.push_back(currentKV);
    }
}



// Handle flushing memtable to Level 1
void LSMTree::flushMemtableToLevel1() {
    // Get all key-value pairs from the memtable
    std::vector<KeyValueWrapper> kvPairs = memtable->flush();
    // cout << "LSMTree::flushMemtableToLevel1()" << endl;

    // Create a new SSTable (DiskBTree) from the kvPairs
    // Generate a unique SSTable file name
    std::string sstableFileName = generateSSTableFileName(1); // Level 1
    fs::path sstablePath = dbPath / sstableFileName;

    // Create a new DiskBTree with the kvPairs
    std::shared_ptr<DiskBTree> newSSTable = std::make_shared<DiskBTree>(sstablePath.string(), kvPairs);

    // If Level 1 is empty, add the new SSTable
    if (levels.empty() || !levels[0]) {
        if (levels.empty()) {
            levels.push_back(newSSTable);
            // Initialize Level 1 capacity if not already
            if (levelMaxSizes.size() < 1) {
                levelMaxSizes.push_back(memtable->getThreshold());
            }
        } else {
            levels[0] = newSSTable;
        }
    } else {
        // Level 1 has an existing SSTable; merge is needed
        levels[0] = newSSTable;
        mergeLevels(1);
    }
}

// Merge SSTables when a level exceeds its capacity
void LSMTree::mergeLevels(int level) {
    // Ensure levels vector is large enough
    if (levels.size() <= static_cast<size_t>(level)) {
        levels.resize(level + 1);
        // Initialize capacity for new level
        size_t newLevelCapacity = levelMaxSizes[level - 1] * 2;
        levelMaxSizes.push_back(newLevelCapacity);
    }

    // Get SSTables to merge
    std::shared_ptr<DiskBTree> sst1 = levels[level - 1]; // Level N
    std::shared_ptr<DiskBTree> sst2 = levels[level];     // Level N+1

    // If Level N+1 is empty, move sst1 up
    if (!sst2) {
        levels[level] = sst1;
        levels[level - 1] = nullptr;
        return;
    }

    // Generate a new SSTable file name
    std::string newSSTableFileName = generateSSTableFileName(level + 1);
    fs::path newSSTablePath = dbPath / newSSTableFileName;

    // Name for the merged leaf pages file
    std::string mergedLeafsFileName = "merge_" + newSSTableFileName + ".leafs";
    fs::path mergedLeafsPath = dbPath / mergedLeafsFileName;

    // Vector to hold the smallest keys of each leaf page
    std::vector<KeyValueWrapper> leafPageSmallestKeys;

    // Merge sst1 and sst2 into mergedLeafsFileName, and populate leafPageSmallestKeys
    mergeSSTables(sst1, sst2, mergedLeafsPath.string(), leafPageSmallestKeys);

    // Create a new DiskBTree instance for the merged SSTable using the new constructor
    std::shared_ptr<DiskBTree> mergedSSTable = std::make_shared<DiskBTree>(
        newSSTablePath.string(), mergedLeafsPath.string(), leafPageSmallestKeys);

    // Update levels
    levels[level] = mergedSSTable;
    levels[level - 1] = nullptr;

    // Delete old SSTable files and merged leaf pages file
    fs::remove(sst1->getFileName());
    fs::remove(sst2->getFileName());
    fs::remove(mergedLeafsPath);

    // Check if the next level exceeds capacity
    if (level + 1 >= static_cast<int>(levels.size())) {
        // Initialize capacity for new level
        size_t newLevelCapacity = levelMaxSizes[level] * 2;
        levelMaxSizes.push_back(newLevelCapacity);
    } else {
        // Check if mergedSSTable exceeds capacity
        if (mergedSSTable->getNumberOfKeyValues() >= levelMaxSizes[level]) {
            // If it exceeds the capacity, recursively merge up
            mergeLevels(level + 1);
        }
    }
}

// Merge two SSTables into a new SSTable
void LSMTree::mergeSSTables(const std::shared_ptr<DiskBTree>& sst1,
                            const std::shared_ptr<DiskBTree>& sst2,
                            const std::string& mergedLeafsFileName,
                            std::vector<KeyValueWrapper>& leafPageSmallestKeys) {
    // Open the output file for writing leaf pages
    PageManager outputLeafPageManager(mergedLeafsFileName);

    // Initialize input PageManagers for sst1 and sst2
    PageManager& pm1 = sst1->pageManager;
    PageManager& pm2 = sst2->pageManager;

    // Get leaf page offsets
    uint64_t sst1LeafBegin = sst1->getLeafBeginOffset();
    uint64_t sst1LeafEnd = sst1->getLeafEndOffset();
    uint64_t sst2LeafBegin = sst2->getLeafBeginOffset();
    uint64_t sst2LeafEnd = sst2->getLeafEndOffset();

    // Initialize iterators over leaf pages
    uint64_t sst1CurrentOffset = sst1LeafBegin;
    uint64_t sst2CurrentOffset = sst2LeafBegin;

    // Buffers to hold entries from each SSTable
    std::vector<KeyValueWrapper> buffer1;
    std::vector<KeyValueWrapper> buffer2;

    // Initialize output page as a pointer
    Page* outputPage = new Page(Page::PageType::LEAF_NODE);

    // Build a bloom filter for the leaf page
    size_t m = 1024; // Number of bits in bloom filter, can be adjusted
    size_t n = 100;  // Expected number of elements, can be adjusted
    outputPage->buildLeafBloomFilter(m, n);

    size_t estimatedPageSize = outputPage->getBaseSize(); // Base size of the page

    size_t pageSize = outputLeafPageManager.getPageSize(); // Assuming same page size

    // Flags to indicate if there are more pages to read
    bool sst1HasMore = true;
    bool sst2HasMore = true;

    // Read the first page from sst1 if available
    if (sst1CurrentOffset <= sst1LeafEnd) {
        Page page1 = pm1.readPage(sst1CurrentOffset);
        buffer1 = page1.getLeafEntries();
        sst1CurrentOffset += pageSize; // Increment to point to the next page
    } else {
        sst1HasMore = false;
    }

    // Read the first page from sst2 if available
    if (sst2CurrentOffset <= sst2LeafEnd) {
        Page page2 = pm2.readPage(sst2CurrentOffset);
        buffer2 = page2.getLeafEntries();
        sst2CurrentOffset += pageSize; // Increment to point to the next page
    } else {
        sst2HasMore = false;
    }

    // Iterators over buffer1 and buffer2
    auto it1 = buffer1.begin();
    auto it2 = buffer2.begin();

    while ((it1 != buffer1.end() || sst1HasMore) || (it2 != buffer2.end() || sst2HasMore)) {
        // Refill buffer1 if exhausted and more pages are available
        if (it1 == buffer1.end() && sst1HasMore) {
            if (sst1CurrentOffset <= sst1LeafEnd) {
                Page page1 = pm1.readPage(sst1CurrentOffset);
                buffer1 = page1.getLeafEntries();
                it1 = buffer1.begin();
                sst1CurrentOffset += pageSize; // Move to the next page
            } else {
                sst1HasMore = false;
            }
        }

        // Refill buffer2 if exhausted and more pages are available
        if (it2 == buffer2.end() && sst2HasMore) {
            if (sst2CurrentOffset <= sst2LeafEnd) {
                Page page2 = pm2.readPage(sst2CurrentOffset);
                buffer2 = page2.getLeafEntries();
                it2 = buffer2.begin();
                sst2CurrentOffset += pageSize; // Move to the next page
            } else {
                sst2HasMore = false;
            }
        }

        // Decide which kv to take next
        KeyValueWrapper nextKV;
        bool haveNextKV = false;

        if (it1 != buffer1.end() && it2 != buffer2.end()) {
            if (*it1 < *it2) {
                nextKV = *it1;
                ++it1;
                haveNextKV = true;
            } else if (*it2 < *it1) {
                nextKV = *it2;
                ++it2;
                haveNextKV = true;
            } else {
                // Keys are equal, resolve based on sequenceNumber and tombstone
                if (it1->sequenceNumber >= it2->sequenceNumber) {
                    nextKV = *it1;
                } else {
                    nextKV = *it2;
                }
                ++it1;
                ++it2;
                haveNextKV = true;
            }
        } else if (it1 != buffer1.end()) {
            nextKV = *it1;
            ++it1;
            haveNextKV = true;
        } else if (it2 != buffer2.end()) {
            nextKV = *it2;
            ++it2;
            haveNextKV = true;
        }

        if (haveNextKV) {
            // Estimate the size of the kv pair
            size_t kvSize = nextKV.getSerializedSize();

            if (estimatedPageSize + kvSize > pageSize) {
                // Page size limit reached, flush current page

                // Record the smallest key in this leaf page
                if (!outputPage->getLeafEntries().empty()) {
                    leafPageSmallestKeys.push_back(outputPage->getLeafEntries().front());
                }

                // Write outputPage to 'merge.leafs'
                uint64_t outputPageOffset = outputLeafPageManager.allocatePage();
                outputLeafPageManager.writePage(outputPageOffset, *outputPage);

                // Delete current outputPage and create a new one
                delete outputPage;
                outputPage = new Page(Page::PageType::LEAF_NODE);
                outputPage->buildLeafBloomFilter(m, n); // Rebuild bloom filter for the new page
                estimatedPageSize = outputPage->getBaseSize();
            }

            // Add kv to outputPage
            outputPage->addLeafEntry(nextKV);
            outputPage->addToLeafBloomFilter(nextKV);
            estimatedPageSize += kvSize;
        }
    }

    // Write any remaining kvs in outputPage
    if (!outputPage->getLeafEntries().empty()) {
        // Record the smallest key in this leaf page
        leafPageSmallestKeys.push_back(outputPage->getLeafEntries().front());

        // Write outputPage to 'merge.leafs'
        uint64_t outputPageOffset = outputLeafPageManager.allocatePage();
        outputLeafPageManager.writePage(outputPageOffset, *outputPage);
    }

    // Clean up
    delete outputPage;

    // Close the output PageManager
    outputLeafPageManager.close();
}

// Generate unique SSTable file names
std::string LSMTree::generateSSTableFileName(int level) {
    static int sstableCounter = 0;
    return "L" + std::to_string(level) + "_SSTable_" + std::to_string(sstableCounter++) + ".sst";
}