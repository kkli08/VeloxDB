//
// Created by damian on 9/24/24.
//

//
// Page.cpp
//

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

// Get leaf node entries
const std::vector<KeyValueWrapper>& Page::getLeafEntries() const {
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

// Serialize the page to a byte buffer
std::vector<char> Page::serialize() const {
    std::vector<char> buffer;
    buffer.reserve(4096); // Assuming 4KB page size

    // Serialize common header
    buffer.push_back(static_cast<uint8_t>(pageType));

    // Serialize based on page type
    switch (pageType) {
        case PageType::INTERNAL_NODE:
            serializeInternalNode(buffer);
            break;
        case PageType::LEAF_NODE:
            serializeLeafNode(buffer);
            break;
        case PageType::SST_METADATA:
            serializeSSTMetadata(buffer);
            break;
        default:
            throw std::logic_error("Unknown page type during serialization");
    }

    // Pad the buffer to page size
    if (buffer.size() < 4096) {
        buffer.resize(4096, 0);
    }

    return buffer;
}

// Deserialize the page from a byte buffer
void Page::deserialize(const std::vector<char>& buffer) {
    if (buffer.empty()) {
        throw std::invalid_argument("Cannot deserialize from empty buffer");
    }

    // Deserialize common header
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

    // Serialize number of child offsets (should be numKeys + 1)
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

// Serialization for Leaf Node
void Page::serializeLeafNode(std::vector<char>& buffer) const {
    // Serialize number of entries
    uint16_t numPairs = static_cast<uint16_t>(leafNodeData.keyValues.size());
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&numPairs), reinterpret_cast<const char*>(&numPairs) + sizeof(numPairs));

    // Serialize key-value pairs
    for (const auto& kv : leafNodeData.keyValues) {
        std::string kvData;
        kv.kv.SerializeToString(&kvData);
        uint32_t kvSize = static_cast<uint32_t>(kvData.size());
        buffer.insert(buffer.end(), reinterpret_cast<const char*>(&kvSize), reinterpret_cast<const char*>(&kvSize) + sizeof(kvSize));
        buffer.insert(buffer.end(), kvData.begin(), kvData.end());
    }

    // Serialize next leaf offset
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&leafNodeData.nextLeafOffset), reinterpret_cast<const char*>(&leafNodeData.nextLeafOffset) + sizeof(leafNodeData.nextLeafOffset));
}

// Serialization for SST Metadata
void Page::serializeSSTMetadata(std::vector<char>& buffer) const {
    // Serialize root offset
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&sstMetadata.rootPageOffset), reinterpret_cast<const char*>(&sstMetadata.rootPageOffset) + sizeof(sstMetadata.rootPageOffset));

    // Serialize leaf node begin and end offsets
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&sstMetadata.leafNodeBeginOffset), reinterpret_cast<const char*>(&sstMetadata.leafNodeBeginOffset) + sizeof(sstMetadata.leafNodeBeginOffset));
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&sstMetadata.leafNodeEndOffset), reinterpret_cast<const char*>(&sstMetadata.leafNodeEndOffset) + sizeof(sstMetadata.leafNodeEndOffset));

    // Serialize file name
    uint32_t nameSize = static_cast<uint32_t>(sstMetadata.fileName.size());
    buffer.insert(buffer.end(), reinterpret_cast<const char*>(&nameSize), reinterpret_cast<const char*>(&nameSize) + sizeof(nameSize));
    buffer.insert(buffer.end(), sstMetadata.fileName.begin(), sstMetadata.fileName.end());
}

// Deserialization for Internal Node
void Page::deserializeInternalNode(const std::vector<char>& buffer) {
    size_t offset = 1; // Start after page type

    // Ensure buffer has enough data for numKeys and numChildOffsets
    if (offset + 2 * sizeof(uint16_t) > buffer.size()) {
        throw std::runtime_error("Buffer too small to read numKeys and numChildOffsets in internal node");
    }

    // Deserialize number of keys
    uint16_t numKeys;
    std::memcpy(&numKeys, &buffer[offset], sizeof(numKeys));
    offset += sizeof(numKeys);
    numEntries = numKeys;

    // Deserialize number of child offsets
    uint16_t numChildOffsets;
    std::memcpy(&numChildOffsets, &buffer[offset], sizeof(numChildOffsets));
    offset += sizeof(numChildOffsets);

    // Ensure buffer has enough data for child offsets
    if (offset + numChildOffsets * sizeof(uint64_t) > buffer.size()) {
        throw std::runtime_error("Buffer too small to read childOffsets in internal node");
    }

    // Deserialize child offsets
    for (uint16_t i = 0; i < numChildOffsets; ++i) {
        uint64_t childOffset;
        std::memcpy(&childOffset, &buffer[offset], sizeof(childOffset));
        offset += sizeof(childOffset);
        internalNodeData.childOffsets.push_back(childOffset);
    }

    // Deserialize keys
    for (uint16_t i = 0; i < numKeys; ++i) {
        // Ensure buffer has enough data for keySize
        if (offset + sizeof(uint32_t) > buffer.size()) {
            throw std::runtime_error("Buffer too small to read keySize in internal node");
        }

        uint32_t keySize;
        std::memcpy(&keySize, &buffer[offset], sizeof(keySize));
        offset += sizeof(keySize);

        // Ensure buffer has enough data for keyData
        if (offset + keySize > buffer.size()) {
            throw std::runtime_error("Buffer too small to read keyData in internal node");
        }

        std::string keyData(buffer.begin() + offset, buffer.begin() + offset + keySize);
        offset += keySize;

        KeyValueWrapper key;
        if (!key.kv.ParseFromString(keyData)) {
            throw std::runtime_error("Failed to parse KeyValueWrapper in internal node");
        }
        internalNodeData.keys.push_back(key);
    }
}

// Deserialization for Leaf Node
void Page::deserializeLeafNode(const std::vector<char>& buffer) {
    size_t offset = 1; // Start after page type

    // Ensure buffer has enough data for numPairs
    if (offset + sizeof(uint16_t) > buffer.size()) {
        throw std::runtime_error("Buffer too small to read numPairs in leaf node");
    }

    // Deserialize number of entries
    uint16_t numPairs;
    std::memcpy(&numPairs, &buffer[offset], sizeof(numPairs));
    offset += sizeof(numPairs);
    numEntries = numPairs;

    // Deserialize key-value pairs
    for (uint16_t i = 0; i < numPairs; ++i) {
        // Ensure buffer has enough data for kvSize
        if (offset + sizeof(uint32_t) > buffer.size()) {
            throw std::runtime_error("Buffer too small to read kvSize in leaf node");
        }

        uint32_t kvSize;
        std::memcpy(&kvSize, &buffer[offset], sizeof(kvSize));
        offset += sizeof(kvSize);

        // Ensure buffer has enough data for kvData
        if (offset + kvSize > buffer.size()) {
            throw std::runtime_error("Buffer too small to read kvData in leaf node");
        }

        std::string kvData(buffer.begin() + offset, buffer.begin() + offset + kvSize);
        offset += kvSize;

        KeyValueWrapper kv;
        if (!kv.kv.ParseFromString(kvData)) {
            throw std::runtime_error("Failed to parse KeyValueWrapper in leaf node");
        }
        leafNodeData.keyValues.push_back(kv);
    }

    // Ensure buffer has enough data for nextLeafOffset
    if (offset + sizeof(uint64_t) > buffer.size()) {
        throw std::runtime_error("Buffer too small to read nextLeafOffset in leaf node");
    }

    std::memcpy(&leafNodeData.nextLeafOffset, &buffer[offset], sizeof(leafNodeData.nextLeafOffset));
    offset += sizeof(leafNodeData.nextLeafOffset);
}

// Deserialization for SST Metadata
void Page::deserializeSSTMetadata(const std::vector<char>& buffer) {
    size_t offset = 1; // Start after page type

    // Ensure buffer has enough data for rootOffset, leafBegin, and leafEnd
    if (offset + 3 * sizeof(uint64_t) > buffer.size()) {
        throw std::runtime_error("Buffer too small to read SST metadata offsets");
    }

    // Deserialize root offset
    std::memcpy(&sstMetadata.rootPageOffset, &buffer[offset], sizeof(sstMetadata.rootPageOffset));
    offset += sizeof(sstMetadata.rootPageOffset);

    // Deserialize leaf node begin and end offsets
    std::memcpy(&sstMetadata.leafNodeBeginOffset, &buffer[offset], sizeof(sstMetadata.leafNodeBeginOffset));
    offset += sizeof(sstMetadata.leafNodeBeginOffset);

    std::memcpy(&sstMetadata.leafNodeEndOffset, &buffer[offset], sizeof(sstMetadata.leafNodeEndOffset));
    offset += sizeof(sstMetadata.leafNodeEndOffset);

    // Ensure buffer has enough data for nameSize
    if (offset + sizeof(uint32_t) > buffer.size()) {
        throw std::runtime_error("Buffer too small to read fileName size in SST metadata");
    }

    // Deserialize file name
    uint32_t nameSize;
    std::memcpy(&nameSize, &buffer[offset], sizeof(nameSize));
    offset += sizeof(nameSize);

    // Ensure buffer has enough data for fileName
    if (offset + nameSize > buffer.size()) {
        throw std::runtime_error("Buffer too small to read fileName in SST metadata");
    }

    sstMetadata.fileName.assign(buffer.begin() + offset, buffer.begin() + offset + nameSize);
    offset += nameSize;
}



