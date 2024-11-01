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
TBD

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

### **LSM-Tree**
TBD