//
// Created by damian on 9/24/24.
//

//
// Page.h
//

#ifndef PAGE_H
#define PAGE_H

#include "KeyValue.h"
#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>

class Page {
public:
    enum class PageType : uint8_t {
        INTERNAL_NODE = 0,
        LEAF_NODE = 1,
        SST_METADATA = 2
        // Other types can be added later
    };

    // Constructor for different page types
    Page(PageType type);

    // Serialize the page to a byte buffer
    std::vector<char> serialize() const;

    // Deserialize the page from a byte buffer
    void deserialize(const std::vector<char>& buffer);

    // Accessors
    PageType getPageType() const { return pageType; }

    // Internal Node specific methods
    void addKey(const KeyValueWrapper& key);
    void addChildOffset(uint64_t childOffset);
    const std::vector<KeyValueWrapper>& getInternalKeys() const;
    const std::vector<uint64_t>& getChildOffsets() const;

    // Leaf Node specific methods
    void addLeafEntry(const KeyValueWrapper& kv);
    const std::vector<KeyValueWrapper>& getLeafEntries() const;
    void setNextLeafOffset(uint64_t offset);
    uint64_t getNextLeafOffset() const;

    // SST Metadata specific methods
    void setMetadata(uint64_t rootOffset, uint64_t leafBegin, uint64_t leafEnd, const std::string& fileName);
    void getMetadata(uint64_t& rootOffset, uint64_t& leafBegin, uint64_t& leafEnd, std::string& fileName) const;

private:
    // Common attributes
    PageType pageType;
    uint16_t numEntries; // Number of keys or key-value pairs

    // For Internal Node Pages
    struct InternalNodeData {
        std::vector<KeyValueWrapper> keys;
        std::vector<uint64_t> childOffsets; // Offsets to child pages, size = keys.size() + 1
    } internalNodeData;

    // For Leaf Node Pages
    struct LeafNodeData {
        std::vector<KeyValueWrapper> keyValues;
        uint64_t nextLeafOffset; // Offset to next leaf node
    } leafNodeData;

    // For SST Metadata Page
    struct SSTMetadata {
        uint64_t rootPageOffset;
        uint64_t leafNodeBeginOffset;
        uint64_t leafNodeEndOffset;
        std::string fileName;
    } sstMetadata;

    // Helper methods for serialization
    void serializeInternalNode(std::vector<char>& buffer) const;
    void serializeLeafNode(std::vector<char>& buffer) const;
    void serializeSSTMetadata(std::vector<char>& buffer) const;

    void deserializeInternalNode(const std::vector<char>& buffer);
    void deserializeLeafNode(const std::vector<char>& buffer);
    void deserializeSSTMetadata(const std::vector<char>& buffer);
};

#endif // PAGE_H

