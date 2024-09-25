//
// Created by damian on 9/24/24.
//
//
// PageTest.cpp
//

#include <gtest/gtest.h>
#include "Page.h"
#include "KeyValue.h"
#include <vector>
#include <string>

// Test the constructor for each PageType
TEST(PageTest, ConstructorTest) {
    // Test creating an Internal Node page
    Page internalPage(Page::PageType::INTERNAL_NODE);
    EXPECT_EQ(internalPage.getPageType(), Page::PageType::INTERNAL_NODE);

    // Test creating a Leaf Node page
    Page leafPage(Page::PageType::LEAF_NODE);
    EXPECT_EQ(leafPage.getPageType(), Page::PageType::LEAF_NODE);

    // Test creating an SST Metadata page
    Page metadataPage(Page::PageType::SST_METADATA);
    EXPECT_EQ(metadataPage.getPageType(), Page::PageType::SST_METADATA);
}

// Test adding keys and child offsets to an internal node and retrieving them
TEST(PageTest, InternalNodeAddAndGetEntries) {
    Page internalPage(Page::PageType::INTERNAL_NODE);

    // Add child offsets and keys
    internalPage.addChildOffset(1000); // C0
    internalPage.addKey(KeyValueWrapper(1, 100)); // K1
    internalPage.addChildOffset(2000); // C1
    internalPage.addKey(KeyValueWrapper(2, 200)); // K2
    internalPage.addChildOffset(3000); // C2

    const std::vector<KeyValueWrapper>& keys = internalPage.getInternalKeys();
    const std::vector<uint64_t>& childOffsets = internalPage.getChildOffsets();

    ASSERT_EQ(keys.size(), 2);
    ASSERT_EQ(childOffsets.size(), 3);

    EXPECT_EQ(childOffsets[0], 1000);
    EXPECT_EQ(keys[0].kv.int_key(), 1);
    EXPECT_EQ(childOffsets[1], 2000);
    EXPECT_EQ(keys[1].kv.int_key(), 2);
    EXPECT_EQ(childOffsets[2], 3000);
}

// Test adding leaf entries and retrieving them
TEST(PageTest, LeafNodeAddAndGetEntries) {
    Page leafPage(Page::PageType::LEAF_NODE);

    // Add leaf entries
    KeyValueWrapper kv1(1, 100);
    KeyValueWrapper kv2(2, 200);
    leafPage.addLeafEntry(kv1);
    leafPage.addLeafEntry(kv2);

    const std::vector<KeyValueWrapper>& keyValues = leafPage.getLeafEntries();

    ASSERT_EQ(keyValues.size(), 2);

    EXPECT_EQ(keyValues[0].kv.int_key(), 1);
    EXPECT_EQ(keyValues[0].kv.int_value(), 100);

    EXPECT_EQ(keyValues[1].kv.int_key(), 2);
    EXPECT_EQ(keyValues[1].kv.int_value(), 200);
}

// Test setting and getting next leaf offset
TEST(PageTest, LeafNodeNextLeafOffset) {
    Page leafPage(Page::PageType::LEAF_NODE);

    // Set next leaf offset
    leafPage.setNextLeafOffset(1234);

    uint64_t offset = leafPage.getNextLeafOffset();
    EXPECT_EQ(offset, 1234);
}

// Test setting and getting metadata
TEST(PageTest, MetadataSetAndGet) {
    Page metadataPage(Page::PageType::SST_METADATA);

    uint64_t rootOffset = 100;
    uint64_t leafBegin = 200;
    uint64_t leafEnd = 300;
    std::string fileName = "sst_1.sst";

    metadataPage.setMetadata(rootOffset, leafBegin, leafEnd, fileName);

    uint64_t rootOffsetRetrieved, leafBeginRetrieved, leafEndRetrieved;
    std::string fileNameRetrieved;

    metadataPage.getMetadata(rootOffsetRetrieved, leafBeginRetrieved, leafEndRetrieved, fileNameRetrieved);

    EXPECT_EQ(rootOffsetRetrieved, rootOffset);
    EXPECT_EQ(leafBeginRetrieved, leafBegin);
    EXPECT_EQ(leafEndRetrieved, leafEnd);
    EXPECT_EQ(fileNameRetrieved, fileName);
}

// Test serialization and deserialization of Internal Node
TEST(PageTest, InternalNodeSerializeDeserialize) {
    Page internalPage(Page::PageType::INTERNAL_NODE);

    // Add child offsets and keys
    internalPage.addChildOffset(1000); // C0
    internalPage.addKey(KeyValueWrapper(1, 100)); // K1
    internalPage.addChildOffset(2000); // C1
    internalPage.addKey(KeyValueWrapper(2, 200)); // K2
    internalPage.addChildOffset(3000); // C2

    // Serialize
    std::vector<char> buffer = internalPage.serialize();

    // Deserialize into a new page
    Page deserializedPage(Page::PageType::INTERNAL_NODE);
    deserializedPage.deserialize(buffer);

    // Verify
    EXPECT_EQ(deserializedPage.getPageType(), Page::PageType::INTERNAL_NODE);

    const std::vector<KeyValueWrapper>& keys = deserializedPage.getInternalKeys();
    const std::vector<uint64_t>& childOffsets = deserializedPage.getChildOffsets();

    ASSERT_EQ(keys.size(), 2);
    ASSERT_EQ(childOffsets.size(), 3);

    EXPECT_EQ(childOffsets[0], 1000);
    EXPECT_EQ(keys[0].kv.int_key(), 1);
    EXPECT_EQ(childOffsets[1], 2000);
    EXPECT_EQ(keys[1].kv.int_key(), 2);
    EXPECT_EQ(childOffsets[2], 3000);
}

// Test serialization and deserialization of Leaf Node
TEST(PageTest, LeafNodeSerializeDeserialize) {
    Page leafPage(Page::PageType::LEAF_NODE);

    // Add leaf entries
    KeyValueWrapper kv1(1, 100);
    KeyValueWrapper kv2(2, 200);
    leafPage.addLeafEntry(kv1);
    leafPage.addLeafEntry(kv2);

    // Set next leaf offset
    leafPage.setNextLeafOffset(1234);

    // Serialize
    std::vector<char> buffer = leafPage.serialize();

    // Deserialize into a new page
    Page deserializedPage(Page::PageType::LEAF_NODE);
    deserializedPage.deserialize(buffer);

    // Verify
    EXPECT_EQ(deserializedPage.getPageType(), Page::PageType::LEAF_NODE);

    const std::vector<KeyValueWrapper>& keyValues = deserializedPage.getLeafEntries();

    ASSERT_EQ(keyValues.size(), 2);

    EXPECT_EQ(keyValues[0].kv.int_key(), 1);
    EXPECT_EQ(keyValues[0].kv.int_value(), 100);

    EXPECT_EQ(keyValues[1].kv.int_key(), 2);
    EXPECT_EQ(keyValues[1].kv.int_value(), 200);

    EXPECT_EQ(deserializedPage.getNextLeafOffset(), 1234);
}

// Test serialization and deserialization of SST Metadata
TEST(PageTest, MetadataSerializeDeserialize) {
    Page metadataPage(Page::PageType::SST_METADATA);

    uint64_t rootOffset = 100;
    uint64_t leafBegin = 200;
    uint64_t leafEnd = 300;
    std::string fileName = "sst_1.sst";

    metadataPage.setMetadata(rootOffset, leafBegin, leafEnd, fileName);

    // Serialize
    std::vector<char> buffer = metadataPage.serialize();

    // Deserialize into a new page
    Page deserializedPage(Page::PageType::SST_METADATA);
    deserializedPage.deserialize(buffer);

    // Verify
    EXPECT_EQ(deserializedPage.getPageType(), Page::PageType::SST_METADATA);

    uint64_t rootOffsetRetrieved, leafBeginRetrieved, leafEndRetrieved;
    std::string fileNameRetrieved;

    deserializedPage.getMetadata(rootOffsetRetrieved, leafBeginRetrieved, leafEndRetrieved, fileNameRetrieved);

    EXPECT_EQ(rootOffsetRetrieved, rootOffset);
    EXPECT_EQ(leafBeginRetrieved, leafBegin);
    EXPECT_EQ(leafEndRetrieved, leafEnd);
    EXPECT_EQ(fileNameRetrieved, fileName);
}

// Test adding internal entry to a non-internal page (should throw exception)
TEST(PageTest, AddInternalEntryToLeafNode) {
    Page leafPage(Page::PageType::LEAF_NODE);

    KeyValueWrapper key(1, 100);
    EXPECT_THROW(leafPage.addKey(key), std::logic_error);
    EXPECT_THROW(leafPage.addChildOffset(1000), std::logic_error);
}

// Test adding leaf entry to a non-leaf page (should throw exception)
TEST(PageTest, AddLeafEntryToInternalNode) {
    Page internalPage(Page::PageType::INTERNAL_NODE);

    KeyValueWrapper kv(1, 100);
    EXPECT_THROW(internalPage.addLeafEntry(kv), std::logic_error);
}

// Test setting metadata on a non-metadata page (should throw exception)
TEST(PageTest, SetMetadataOnNonMetadataPage) {
    Page leafPage(Page::PageType::LEAF_NODE);

    EXPECT_THROW(leafPage.setMetadata(100, 200, 300, "sst_1.sst"), std::logic_error);
}

// Test getting metadata from a non-metadata page (should throw exception)
TEST(PageTest, GetMetadataFromNonMetadataPage) {
    Page internalPage(Page::PageType::INTERNAL_NODE);

    uint64_t rootOffset, leafBegin, leafEnd;
    std::string fileName;
    EXPECT_THROW(internalPage.getMetadata(rootOffset, leafBegin, leafEnd, fileName), std::logic_error);
}

// Test deserializing from an empty buffer (should throw exception)
TEST(PageTest, DeserializeFromEmptyBuffer) {
    Page page(Page::PageType::INTERNAL_NODE);
    std::vector<char> emptyBuffer;
    EXPECT_THROW(page.deserialize(emptyBuffer), std::invalid_argument);
}

// Test serialization and deserialization of an empty Internal Node page
TEST(PageTest, EmptyInternalNodeSerializeDeserialize) {
    Page internalPage(Page::PageType::INTERNAL_NODE);

    // Add initial child offset (since internal nodes should have at least one child offset)
    internalPage.addChildOffset(1000);

    // Serialize
    std::vector<char> buffer = internalPage.serialize();

    // Deserialize
    Page deserializedPage(Page::PageType::INTERNAL_NODE);
    deserializedPage.deserialize(buffer);

    EXPECT_EQ(deserializedPage.getPageType(), Page::PageType::INTERNAL_NODE);

    const auto& keys = deserializedPage.getInternalKeys();
    const auto& childOffsets = deserializedPage.getChildOffsets();

    EXPECT_TRUE(keys.empty());
    ASSERT_EQ(childOffsets.size(), 1);
    EXPECT_EQ(childOffsets[0], 1000);
}

// Test serialization and deserialization of an empty Leaf Node page
TEST(PageTest, EmptyLeafNodeSerializeDeserialize) {
    Page leafPage(Page::PageType::LEAF_NODE);

    // Serialize
    std::vector<char> buffer = leafPage.serialize();

    // Deserialize
    Page deserializedPage(Page::PageType::LEAF_NODE);
    deserializedPage.deserialize(buffer);

    EXPECT_EQ(deserializedPage.getPageType(), Page::PageType::LEAF_NODE);

    const auto& keyValues = deserializedPage.getLeafEntries();

    EXPECT_TRUE(keyValues.empty());

    EXPECT_EQ(deserializedPage.getNextLeafOffset(), 0);
}

// Test serialization and deserialization with various key types
TEST(PageTest, LeafNodeWithVariousKeyTypesSerializeDeserialize) {
    Page leafPage(Page::PageType::LEAF_NODE);

    KeyValueWrapper kvInt(1, 100);
    KeyValueWrapper kvDouble(3.14, 1.618);
    KeyValueWrapper kvString("key1", "value1");
    KeyValueWrapper kvChar('A', 'Z');

    leafPage.addLeafEntry(kvInt);
    leafPage.addLeafEntry(kvDouble);
    leafPage.addLeafEntry(kvString);
    leafPage.addLeafEntry(kvChar);

    // Serialize
    std::vector<char> buffer = leafPage.serialize();

    // Deserialize
    Page deserializedPage(Page::PageType::LEAF_NODE);
    deserializedPage.deserialize(buffer);

    EXPECT_EQ(deserializedPage.getPageType(), Page::PageType::LEAF_NODE);

    const auto& keyValues = deserializedPage.getLeafEntries();

    ASSERT_EQ(keyValues.size(), 4);

    // Verify integer key-value
    EXPECT_EQ(keyValues[0].kv.int_key(), 1);
    EXPECT_EQ(keyValues[0].kv.int_value(), 100);

    // Verify double key-value
    EXPECT_EQ(keyValues[1].kv.double_key(), 3.14);
    EXPECT_EQ(keyValues[1].kv.double_value(), 1.618);

    // Verify string key-value
    EXPECT_EQ(keyValues[2].kv.string_key(), "key1");
    EXPECT_EQ(keyValues[2].kv.string_value(), "value1");

    // Verify char key-value
    // Assuming kv.char_key() returns std::string
    EXPECT_EQ(keyValues[3].kv.char_key(), std::string(1, 'A'));
    EXPECT_EQ(keyValues[3].kv.char_value(), std::string(1, 'Z'));
}

// Test serialization and deserialization of a large Leaf Node page
TEST(PageTest, LargeLeafNodeSerializeDeserialize) {
    Page leafPage(Page::PageType::LEAF_NODE);

    // Add many leaf entries
    for (int i = 0; i < 1000; ++i) {
        KeyValueWrapper kv(i, i * 10);
        leafPage.addLeafEntry(kv);
    }

    // Set next leaf offset
    leafPage.setNextLeafOffset(123456);

    // Serialize
    std::vector<char> buffer = leafPage.serialize();

    // Deserialize
    Page deserializedPage(Page::PageType::LEAF_NODE);
    deserializedPage.deserialize(buffer);

    EXPECT_EQ(deserializedPage.getPageType(), Page::PageType::LEAF_NODE);

    const auto& keyValues = deserializedPage.getLeafEntries();

    ASSERT_EQ(keyValues.size(), 1000);

    for (int i = 0; i < 1000; ++i) {
        EXPECT_EQ(keyValues[i].kv.int_key(), i);
        EXPECT_EQ(keyValues[i].kv.int_value(), i * 10);
    }

    EXPECT_EQ(deserializedPage.getNextLeafOffset(), 123456);
}





