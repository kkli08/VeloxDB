### SST Files Layout
```
[SST Metadata Page]
[Leaf Node Page 1]
[Leaf Node Page 2]
[Leaf Node Page 3]
...
[Leaf Node Page m]
[Internal Node Page (Root)]
[Internal Node Page 1]
[Internal Node Page 2]
...
[Internal Node Page n]
[* Clustered Index Page]
[* Bloom Filter Page]
```
#### `Page::PageSize`
> Page with `PageSize::` **PageSize** (`4KB`, `8KB`)

#### `Page::SST_MetaData`
```c++
struct SSTMetadata {
        uint64_t rootPageOffset;
        uint64_t leafNodeBeginOffset;
        uint64_t leafNodeEndOffset;
        std::string fileName;

        // SST Bloom filter
        BloomFilter bloomFilter;
        bool hasBloomFilter = false;
    }
```

#### `Page::LeafNodes`
```c++
/*
 *  4kb / 8kb chunk
 *  sorted by key
 */
serialized key-value pair 1 metadata (serialized by protobuf)
serialized key-value pair 2 metadata (serialized by protobuf)
serialized key-value pair 3 metadata (serialized by protobuf)
...
Bloom Filter for each Leaf Page
// with padding
```
```c++
struct LeafNodeData {
        std::vector<KeyValueWrapper> keyValues;
        uint64_t nextLeafOffset; // Offset to next leaf node
        
        // Bloom filter for the leaf node
        BloomFilter bloomFilter;
        bool hasBloomFilter = false;
    }
```

#### `Page::InternalNodes`
```c++
/*
 *  4kb / 8kb chunk
 *  sorted by level
 */
level#0 key-value pair 0 metadata (serialized by protobuf), jump_offset_L1_K0, jump_offset_L1_K1
level#1 key-value pair 1 metadata (serialized by protobuf), jump_offset_L2_K0, jump_offset_L2_K1
level#1 key-value pair 2 metadata (serialized by protobuf), jump_offset_L2_K1, jump_offset_L2_K2
...
// with padding
```
```c++
struct InternalNodeData {
        std::vector<KeyValueWrapper> keys;
        std::vector<uint64_t> childOffsets; // Offsets to child pages, size = keys.size() + 1
    }
```

#### `Page::BloomFilter`
TBD
#### `Page::ClusteredIndex`
TBD
