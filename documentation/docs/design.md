### **LSM-Tree**
```c++
// LSMTree.h
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

    // Insert a key-value pair into the LSM tree
    void put(const KeyValueWrapper& kv);

    // Search for a key-value pair in the LSM tree
    KeyValueWrapper get(const KeyValueWrapper& kv);

    // Scan method
    void scan(const KeyValueWrapper& startKey, const KeyValueWrapper& endKey, std::vector<KeyValueWrapper>& result);

    // ...

private:
    // Level 0 is always the in-memory memtable
    std::unique_ptr<Memtable> memtable; // Level 0

    // Levels 1 and above consist of DiskBTrees (SSTables)
    // Each level holds at most one SSTable
    std::vector<std::shared_ptr<DiskBTree>> levels; // Levels 1+

    // Size Ratio of the LSM-Tree between levels
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

    // ...
};
```
### **Buffer Pool**
```c++
// PageManager.h
class PageManager {
public:
    // ...

    // BufferPool configuration
    void setBufferPoolParameters(size_t capacity, EvictionPolicy policy);
    long long getCacheHit() const {return bufferPool->getCacheHit();};

    // ...

private:
    // ...
    
    // shared buffer pool among all the pageManager
    std::shared_ptr<BufferPool> bufferPool;
    
    // ...
};

```

### **Bloom Filter**
```c++
// VeloxDB.h

// Set buffer pool parameters
void setBufferPoolParameters(size_t capacity, EvictionPolicy policy);

// Print total CacheHit in the buffer pool
void printCacheHit() const;
```

### **Static B+ Tree as SST file**
#### Writing into sst file
```c++
DiskBTree::DiskBTree(const std::string& sstFileName, const std::vector<KeyValueWrapper>& keyValues, size_t pageSize)
    : sstFileName(sstFileName), pageManager(sstFileName, pageSize), pageSize(pageSize), root(nullptr)
{
    // Constructor for creating a new SST file

    // Step 1: Write placeholder metadata to offset 0
    Page metadataPage(Page::PageType::SST_METADATA);
    pageManager.writePage(0, metadataPage); // Reserve offset 0

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
    pageManager.writePage(0, metadataPage);

    // After writing, clear the in-memory structures to free memory
    // ...
}
```
#### Reading sst file
```c++
DiskBTree::DiskBTree(const std::string& sstFileName)
    : sstFileName(sstFileName), pageManager(sstFileName), root(nullptr)
{
    // Constructor for reading an existing SST file

    // Read the metadata page from offset 0
    Page metadataPage = pageManager.readPage(0);

    // Extract metadata
    std::string fileName;
    metadataPage.getMetadata(rootOffset, leafBeginOffset, leafEndOffset, fileName);

    // sstFileName is already set; ensure it matches the metadata (optional)
    if (sstFileName != fileName) {
        std::cerr << "Warning: SST file name does not match metadata file name." << std::endl;
    }

    // Since the tree is static, we don't load any nodes into memory
    // We rely on reading pages from disk during search and scan operations
}
```

