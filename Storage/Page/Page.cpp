// Page.cpp

#include "Page.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

// Constructor
Page::Page(PageType type) : pageType(type), numEntries(0) {
    if (type == PageType::LEAF_NODE) {
        leafNodeData.nextLeafOffset = 0;
    }
}

Page::Page(): numEntries(0) {}

// Add a key to the internal node
void Page::addKey(const KeyValueWrapper& key) {
    if (pageType != PageType::INTERNAL_NODE) {
        throw std::logic_error("Attempting to add key to non-internal page");
    }
    internalNodeData.keys.push_back(key);
    numEntries++;
}

// Add a child offset to the internal node
void Page::addChildOffset(uint64_t childOffset) {
    if (pageType != PageType::INTERNAL_NODE) {
        throw std::logic_error("Attempting to add child offset to non-internal page");
    }
    internalNodeData.childOffsets.push_back(childOffset);
}

// Get internal node keys
const std::vector<KeyValueWrapper>& Page::getInternalKeys() const {
    return internalNodeData.keys;
}

// Get child offsets
const std::vector<uint64_t>& Page::getChildOffsets() const {
    return internalNodeData.childOffsets;
}

// Add a leaf node entry
void Page::addLeafEntry(const KeyValueWrapper& kv) {
    if (pageType != PageType::LEAF_NODE) {
        throw std::logic_error("Attempting to add leaf entry to non-leaf page");
    }
    leafNodeData.keyValues.push_back(kv);
    numEntries++;
}

// Remove the last leaf entry
void Page::removeLastLeafEntry() {
    if (pageType != PageType::LEAF_NODE) {
        throw std::logic_error("Attempting to remove leaf entry from non-leaf page");
    }
    if (!leafNodeData.keyValues.empty()) {
        leafNodeData.keyValues.pop_back();
        numEntries--;
    } else {
        throw std::runtime_error("No leaf entries to remove");
    }
}

// Get leaf node entries
const std::vector<KeyValueWrapper>& Page::getLeafEntries() const {
    if (pageType != PageType::LEAF_NODE) {
        throw std::logic_error("Attempting to get leaf entries from non-leaf page");
    }
    return leafNodeData.keyValues;
}

// Set next leaf offset
void Page::setNextLeafOffset(uint64_t offset) {
    if (pageType != PageType::LEAF_NODE) {
        throw std::logic_error("Attempting to set next leaf offset on non-leaf page");
    }
    leafNodeData.nextLeafOffset = offset;
}

// Get next leaf offset
uint64_t Page::getNextLeafOffset() const {
    if (pageType != PageType::LEAF_NODE) {
        throw std::logic_error("Attempting to get next leaf offset from non-leaf page");
    }
    return leafNodeData.nextLeafOffset;
}

// Set SST metadata
void Page::setMetadata(uint64_t rootOffset, uint64_t leafBegin, uint64_t leafEnd, const std::string& fileName) {
    if (pageType != PageType::SST_METADATA) {
        throw std::logic_error("Attempting to set metadata on non-metadata page");
    }
    sstMetadata.rootPageOffset = rootOffset;
    sstMetadata.leafNodeBeginOffset = leafBegin;
    sstMetadata.leafNodeEndOffset = leafEnd;
    sstMetadata.fileName = fileName;
}

// Get SST metadata
void Page::getMetadata(uint64_t& rootOffset, uint64_t& leafBegin, uint64_t& leafEnd, std::string& fileName) const {
    if (pageType != PageType::SST_METADATA) {
        throw std::logic_error("Attempting to get metadata from non-metadata page");
    }
    rootOffset = sstMetadata.rootPageOffset;
    leafBegin = sstMetadata.leafNodeBeginOffset;
    leafEnd = sstMetadata.leafNodeEndOffset;
    fileName = sstMetadata.fileName;
}

// Methods to set and get the SST Bloom filter in metadata page
void Page::setSSTBloomFilter(const std::vector<char>& bloomFilterData) {
    if (pageType != PageType::SST_METADATA) {
        throw std::logic_error("Attempting to set SST Bloom filter on non-metadata page");
    }
    sstMetadata.bloomFilter.deserialize(bloomFilterData);
    sstMetadata.hasBloomFilter = true;
}

bool Page::getSSTBloomFilter(std::vector<char>& bloomFilterData) const {
    if (pageType != PageType::SST_METADATA) {
        throw std::logic_error("Attempting to get SST Bloom filter from non-metadata page");
    }
    if (sstMetadata.hasBloomFilter) {
        bloomFilterData = sstMetadata.bloomFilter.serialize();
        return true;
    }
    return false;
}

// Serialize the page to a byte buffer
std::vector<char> Page::serialize() const {
    std::vector<char> buffer;
    buffer.reserve(4096); // Assuming 4KB page size

    // Serialize page type
    buffer.push_back(static_cast<uint8_t>(pageType));

    // Serialize based on page type
    switch (pageType) {
        case PageType::INTERNAL_NODE:
            // cout << "Page::serialize() --> serialize internal node" << endl;
            serializeInternalNode(buffer);
            break;
        case PageType::LEAF_NODE:
            // cout << "Page::serialize() --> serialize leaf node" << endl;
            serializeLeafNode(buffer);
            break;
        case PageType::SST_METADATA:
            // cout << "Page::serialize() --> serialize sst metadata" << endl;
            serializeSSTMetadata(buffer);
            break;
        default:
            throw std::logic_error("Unknown page type during serialization");
    }

    // Pad the buffer to page size
    if (buffer.size() < 4096) {
        buffer.resize(4096, 0);
    }

    // After serializing the page
    if (buffer.size() > DEFAULT_PAGE_SIZE) {
        std::cerr << "Serialized page size: " << buffer.size() << " bytes\n";
        std::cerr << "Serialized page type: "
                  << [](PageType type) {
                      switch (type) {
                          case PageType::INTERNAL_NODE: return "INTERNAL_NODE";
                          case PageType::LEAF_NODE: return "LEAF_NODE";
                          case PageType::SST_METADATA: return "SST_METADATA";
                          default: return "UNKNOWN";
                      }
                    }(pageType)
                 << std::endl;

        throw std::runtime_error("Page::serialize() --> Serialized page exceeds the maximum page size");
    }

    return buffer;
}

// Deserialize the page from a byte buffer
void Page::deserialize(const std::vector<char>& buffer) {
    if (buffer.empty()) {
        throw std::invalid_argument("Cannot deserialize from empty buffer");
    }

    // Deserialize page type
    pageType = static_cast<PageType>(buffer[0]);

    // Deserialize based on page type
    switch (pageType) {
        case PageType::INTERNAL_NODE:
            deserializeInternalNode(buffer);
            break;
        case PageType::LEAF_NODE:
            deserializeLeafNode(buffer);
            break;
        case PageType::SST_METADATA:
            deserializeSSTMetadata(buffer);
            break;
        default:
            throw std::logic_error("Unknown page type during deserialization");
    }
}

// Serialization for Internal Node
void Page::serializeInternalNode(std::vector<char>& buffer) const {
    // Serialize number of keys
    uint16_t numKeys = static_cast<uint16_t>(internalNodeData.keys.size());
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&numKeys), reinterpret_cast<const char*>(&numKeys) + sizeof(numKeys));

    // Serialize number of child offsets
    uint16_t numChildOffsets = static_cast<uint16_t>(internalNodeData.childOffsets.size());
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&numChildOffsets), reinterpret_cast<const char*>(&numChildOffsets) + sizeof(numChildOffsets));

    // Serialize child offsets
    for (const auto& childOffset : internalNodeData.childOffsets) {
        buffer.insert(buffer.end(), reinterpret_cast<const char*>(&childOffset), reinterpret_cast<const char*>(&childOffset) + sizeof(childOffset));
    }

    // Serialize keys
    for (const auto& key : internalNodeData.keys) {
        std::string keyData;
        key.kv.SerializeToString(&keyData);
        uint32_t keySize = static_cast<uint32_t>(keyData.size());
        buffer.insert(buffer.end(), reinterpret_cast<const char*>(&keySize), reinterpret_cast<const char*>(&keySize) + sizeof(keySize));
        buffer.insert(buffer.end(), keyData.begin(), keyData.end());
    }
}

// Deserialization for Internal Node
void Page::deserializeInternalNode(const std::vector<char>& buffer) {
    size_t offset = 1; // Start after page type

    // Deserialize number of keys
    uint16_t numKeys;
    std::memcpy(&numKeys, &buffer[offset], sizeof(numKeys));
    offset += sizeof(numKeys);
    numEntries = numKeys;

    // Deserialize number of child offsets
    uint16_t numChildOffsets;
    std::memcpy(&numChildOffsets, &buffer[offset], sizeof(numChildOffsets));
    offset += sizeof(numChildOffsets);

    // Deserialize child offsets
    for (uint16_t i = 0; i < numChildOffsets; ++i) {
        uint64_t childOffset;
        std::memcpy(&childOffset, &buffer[offset], sizeof(childOffset));
        offset += sizeof(childOffset);
        internalNodeData.childOffsets.push_back(childOffset);
    }

    // Deserialize keys
    for (uint16_t i = 0; i < numKeys; ++i) {
        uint32_t keySize;
        std::memcpy(&keySize, &buffer[offset], sizeof(keySize));
        offset += sizeof(keySize);

        std::string keyData(buffer.begin() + offset, buffer.begin() + offset + keySize);
        offset += keySize;

        KeyValueWrapper key;
        if (!key.kv.ParseFromString(keyData)) {
            throw std::runtime_error("Page::deserializeInternalNode() --> Failed to parse KeyValueWrapper in internal node");
        }
        internalNodeData.keys.push_back(key);
    }
}

// Serialization for Leaf Node
void Page::serializeLeafNode(std::vector<char>& buffer) const {
   // Serialize number of entries
   uint16_t numPairs = static_cast<uint16_t>(leafNodeData.keyValues.size());
   buffer.insert(buffer.end(), reinterpret_cast<const char*>(&numPairs), reinterpret_cast<const char*>(&numPairs) + sizeof(numPairs));

   // Serialize key-value pairs
   for (const auto& kv : leafNodeData.keyValues) {
       // Serialize sequenceNumber
       uint64_t seqNum = kv.sequenceNumber;
       buffer.insert(buffer.end(), reinterpret_cast<const char*>(&seqNum), reinterpret_cast<const char*>(&seqNum) + sizeof(seqNum));

       // Serialize tombstone
       uint8_t tombstoneFlag = kv.tombstone ? 1 : 0;
       buffer.push_back(tombstoneFlag);

       // Serialize KeyValue
       std::string kvData;
       kv.kv.SerializeToString(&kvData);
       uint32_t kvSize = static_cast<uint32_t>(kvData.size());
       buffer.insert(buffer.end(), reinterpret_cast<const char*>(&kvSize), reinterpret_cast<const char*>(&kvSize) + sizeof(kvSize));
       buffer.insert(buffer.end(), kvData.begin(), kvData.end());
   }

   // Serialize next leaf offset
   buffer.insert(buffer.end(), reinterpret_cast<const char*>(&leafNodeData.nextLeafOffset),
                 reinterpret_cast<const char*>(&leafNodeData.nextLeafOffset) + sizeof(leafNodeData.nextLeafOffset));

   // Serialize hasBloomFilter flag
   uint8_t hasBF = leafNodeData.hasBloomFilter ? 1 : 0;
   buffer.push_back(hasBF);

   if (leafNodeData.hasBloomFilter) {
       // Serialize Bloom filter
       std::vector<char> bloomFilterData = leafNodeData.bloomFilter.serialize();
       uint32_t bloomFilterSize32 = static_cast<uint32_t>(bloomFilterData.size());

       // Serialize bloomFilterSize
       buffer.insert(buffer.end(), reinterpret_cast<const char*>(&bloomFilterSize32),
                     reinterpret_cast<const char*>(&bloomFilterSize32) + sizeof(bloomFilterSize32));

       // Serialize Bloom filter data
       buffer.insert(buffer.end(), bloomFilterData.begin(), bloomFilterData.end());
   }
}

// Deserialization for Leaf Node
void Page::deserializeLeafNode(const std::vector<char>& buffer) {
   size_t offset = 1; // Start after page type

   // Deserialize number of entries
   uint16_t numPairs;
   std::memcpy(&numPairs, &buffer[offset], sizeof(numPairs));
   offset += sizeof(numPairs);
   numEntries = numPairs;

   // Deserialize key-value pairs
   for (uint16_t i = 0; i < numPairs; ++i) {
       // Deserialize sequenceNumber
       uint64_t seqNum;
       std::memcpy(&seqNum, &buffer[offset], sizeof(seqNum));
       offset += sizeof(seqNum);

       // Deserialize tombstone
       uint8_t tombstoneFlag = buffer[offset];
       offset += sizeof(uint8_t);

       // Deserialize KeyValue
       uint32_t kvSize;
       std::memcpy(&kvSize, &buffer[offset], sizeof(kvSize));
       offset += sizeof(kvSize);

       std::string kvData(buffer.begin() + offset, buffer.begin() + offset + kvSize);
       offset += kvSize;

       KeyValueWrapper kv;
       if (!kv.kv.ParseFromString(kvData)) {
           throw std::runtime_error("Failed to parse KeyValueWrapper in leaf node");
       }

       kv.sequenceNumber = seqNum;
       kv.tombstone = (tombstoneFlag == 1);

       leafNodeData.keyValues.push_back(kv);
   }

   // Deserialize next leaf offset
   std::memcpy(&leafNodeData.nextLeafOffset, &buffer[offset], sizeof(leafNodeData.nextLeafOffset));
   offset += sizeof(leafNodeData.nextLeafOffset);

   // Deserialize hasBloomFilter flag
   if (offset >= buffer.size()) {
       leafNodeData.hasBloomFilter = false;
       return;
   }

   uint8_t hasBF = buffer[offset];
   offset += sizeof(uint8_t);

   if (hasBF) {
       leafNodeData.hasBloomFilter = true;

       // Deserialize bloomFilterSize
       uint32_t bloomFilterSize;
       std::memcpy(&bloomFilterSize, &buffer[offset], sizeof(bloomFilterSize));
       offset += sizeof(bloomFilterSize);

       // Deserialize Bloom filter data
       std::vector<char> bloomFilterData(buffer.begin() + offset, buffer.begin() + offset + bloomFilterSize);
       offset += bloomFilterSize;

       // Deserialize the Bloom filter
       leafNodeData.bloomFilter.deserialize(bloomFilterData);
   } else {
       leafNodeData.hasBloomFilter = false;
   }
}


// Serialization for SST Metadata
void Page::serializeSSTMetadata(std::vector<char>& buffer) const {
    // Serialize root offset
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&sstMetadata.rootPageOffset),
                  reinterpret_cast<const char*>(&sstMetadata.rootPageOffset) + sizeof(sstMetadata.rootPageOffset));

    // Serialize leaf node begin and end offsets
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&sstMetadata.leafNodeBeginOffset),
                  reinterpret_cast<const char*>(&sstMetadata.leafNodeBeginOffset) + sizeof(sstMetadata.leafNodeBeginOffset));
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&sstMetadata.leafNodeEndOffset),
                  reinterpret_cast<const char*>(&sstMetadata.leafNodeEndOffset) + sizeof(sstMetadata.leafNodeEndOffset));

    // Serialize file name
    uint32_t nameSize = static_cast<uint32_t>(sstMetadata.fileName.size());
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&nameSize),
                  reinterpret_cast<const char*>(&nameSize) + sizeof(nameSize));
    buffer.insert(buffer.end(), sstMetadata.fileName.begin(), sstMetadata.fileName.end());

    // Serialize hasBloomFilter flag
    uint8_t hasBF = sstMetadata.hasBloomFilter ? 1 : 0;
    buffer.push_back(hasBF);

    if (sstMetadata.hasBloomFilter) {
        // Serialize Bloom filter
        std::vector<char> bloomFilterData = sstMetadata.bloomFilter.serialize();
        uint32_t bloomFilterSize = static_cast<uint32_t>(bloomFilterData.size());

        // Serialize bloomFilterSize
        buffer.insert(buffer.end(), reinterpret_cast<const char*>(&bloomFilterSize),
                      reinterpret_cast<const char*>(&bloomFilterSize) + sizeof(bloomFilterSize));

        // Serialize Bloom filter data
        buffer.insert(buffer.end(), bloomFilterData.begin(), bloomFilterData.end());
    }
}

// Deserialization for SST Metadata
void Page::deserializeSSTMetadata(const std::vector<char>& buffer) {
    size_t offset = 1; // Start after page type

    // Deserialize root offset
    std::memcpy(&sstMetadata.rootPageOffset, &buffer[offset], sizeof(sstMetadata.rootPageOffset));
    offset += sizeof(sstMetadata.rootPageOffset);

    // Deserialize leaf node begin and end offsets
    std::memcpy(&sstMetadata.leafNodeBeginOffset, &buffer[offset], sizeof(sstMetadata.leafNodeBeginOffset));
    offset += sizeof(sstMetadata.leafNodeBeginOffset);

    std::memcpy(&sstMetadata.leafNodeEndOffset, &buffer[offset], sizeof(sstMetadata.leafNodeEndOffset));
    offset += sizeof(sstMetadata.leafNodeEndOffset);

    // Deserialize file name
    uint32_t nameSize;
    std::memcpy(&nameSize, &buffer[offset], sizeof(nameSize));
    offset += sizeof(nameSize);

    sstMetadata.fileName.assign(buffer.begin() + offset, buffer.begin() + offset + nameSize);
    offset += nameSize;

    // Deserialize hasBloomFilter flag
    if (offset >= buffer.size()) {
        sstMetadata.hasBloomFilter = false;
        return;
    }

    uint8_t hasBF = buffer[offset];
    offset += sizeof(uint8_t);

    if (hasBF) {
        sstMetadata.hasBloomFilter = true;

        // Deserialize bloomFilterSize
        uint32_t bloomFilterSize;
        std::memcpy(&bloomFilterSize, &buffer[offset], sizeof(bloomFilterSize));
        offset += sizeof(bloomFilterSize);

        // Deserialize Bloom filter data
        if (offset + bloomFilterSize > buffer.size()) {
            throw std::runtime_error("Buffer too small to read Bloom filter data in SST metadata");
        }

        std::vector<char> bloomFilterData(buffer.begin() + offset, buffer.begin() + offset + bloomFilterSize);
        offset += bloomFilterSize;

        // Deserialize the Bloom filter
        sstMetadata.bloomFilter.deserialize(bloomFilterData);
    } else {
        sstMetadata.hasBloomFilter = false;
    }
}

// Build Bloom filter for leaf node
void Page::buildLeafBloomFilter(size_t m, size_t n) {
    if (pageType != PageType::LEAF_NODE) {
        throw std::logic_error("Attempting to build Bloom filter on non-leaf page");
    }
    leafNodeData.bloomFilter = BloomFilter(m, n);
    leafNodeData.hasBloomFilter = true;
}

// Add to leaf Bloom filter
void Page::addToLeafBloomFilter(const KeyValueWrapper& kv) {
    if (pageType != PageType::LEAF_NODE) {
        throw std::logic_error("Attempting to add to Bloom filter on non-leaf page");
    }
    if (!leafNodeData.hasBloomFilter) {
        throw std::runtime_error("Bloom filter has not been initialized");
    }
    leafNodeData.bloomFilter.add(kv);
}

// Check if a key possibly exists in the leaf node
bool Page::leafBloomFilterContains(const KeyValueWrapper& kv) const {
    if (pageType != PageType::LEAF_NODE) {
        throw std::logic_error("Attempting to check Bloom filter on non-leaf page");
    }
    if (!leafNodeData.hasBloomFilter) {
        // If no Bloom filter, assume it might contain the key
        return true;
    }
    // std::cout << "Page::leafBloomFilterContains() called" << std::endl;
    return leafNodeData.bloomFilter.possiblyContains(kv);
}

// Estimate the base size of the page for serialization
size_t Page::getBaseSize() const {
    size_t size = sizeof(PageType) + sizeof(uint16_t); // pageType and numEntries
    switch (pageType) {
        case PageType::INTERNAL_NODE:
            // For internal node, size of numKeys and numChildOffsets
            size += sizeof(uint16_t) * 2;
            break;
        case PageType::LEAF_NODE:
            // For leaf node, size of numPairs and nextLeafOffset, hasBloomFilter flag
            size += sizeof(uint16_t); // numPairs
            size += sizeof(uint64_t); // nextLeafOffset
            size += sizeof(uint8_t); // hasBloomFilter
            if (leafNodeData.hasBloomFilter) {
                size += sizeof(uint32_t); // bloomFilterSize
                size += leafNodeData.bloomFilter.getSerializedSize();
            }
            break;
        case PageType::SST_METADATA:
            // For SST metadata, sizes of offsets and file name length
            size += sizeof(uint64_t) * 3; // rootPageOffset, leafNodeBeginOffset, leafNodeEndOffset
            size += sizeof(uint32_t); // nameSize
            size += sstMetadata.fileName.size(); // fileName
            size += sizeof(uint8_t); // hasBloomFilter
            if (sstMetadata.hasBloomFilter) {
                size += sizeof(uint32_t); // bloomFilterSize
                size += sstMetadata.bloomFilter.getSerializedSize();
            }
            break;
    }
    return size;
}
