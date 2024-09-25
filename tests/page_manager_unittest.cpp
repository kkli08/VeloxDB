//
// Created by damian on 9/24/24.
//
//
// PageManagerTest.cpp
//

#include <gtest/gtest.h>
#include <filesystem>
#include "PageManager.h"
#include "Page.h"
#include "KeyValue.h"

// Test that the constructor creates the file if it doesn't exist
TEST(PageManagerTest, ConstructorCreatesFile) {
    // Ensure test directory exists
    std::filesystem::create_directories("test_db");

    // Define file name
    std::string fileName = "test_db/test_page_manager.dat";

    // Remove file if it exists
    if (std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

    // Create PageManager
    PageManager pageManager(fileName);

    // Check that the file now exists
    EXPECT_TRUE(std::filesystem::exists(fileName));

    // Clean up
    pageManager.close();
    std::filesystem::remove(fileName);
    std::filesystem::remove_all("test_db");
}

// Test that allocatePage returns correct offsets
TEST(PageManagerTest, AllocatePageReturnsCorrectOffsets) {
    // Ensure test directory exists
    std::filesystem::create_directories("test_db");

    std::string fileName = "test_db/test_page_manager.dat";

    // Remove file if it exists
    if (std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

    PageManager pageManager(fileName);

    size_t pageSize = 4096;

    // Allocate pages and check offsets
    uint64_t offset1 = pageManager.allocatePage();
    EXPECT_EQ(offset1, 4096);

    uint64_t offset2 = pageManager.allocatePage();
    EXPECT_EQ(offset2, 2 * pageSize);

    uint64_t offset3 = pageManager.allocatePage();
    EXPECT_EQ(offset3, 3 * pageSize);

    // Clean up
    pageManager.close();
    std::filesystem::remove(fileName);
    std::filesystem::remove_all("test_db");
}

// Test writing and reading a page
TEST(PageManagerTest, WritePageAndReadPage) {
    // Ensure test directory exists
    std::filesystem::create_directories("test_db");

    std::string fileName = "test_db/test_page_manager.dat";

    // Remove file if it exists
    if (std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

    PageManager pageManager(fileName);

    // Allocate a page
    uint64_t offset = pageManager.allocatePage();

    // Create a page
    Page page(Page::PageType::LEAF_NODE);
    KeyValueWrapper kv1(1, 100);
    KeyValueWrapper kv2(2, 200);
    page.addLeafEntry(kv1);
    page.addLeafEntry(kv2);
    page.setNextLeafOffset(0);

    // Write the page
    pageManager.writePage(offset, page);

    // Read the page back
    Page readPage = pageManager.readPage(offset);

    // Verify the page content
    EXPECT_EQ(readPage.getPageType(), Page::PageType::LEAF_NODE);

    const auto& keyValues = readPage.getLeafEntries();
    ASSERT_EQ(keyValues.size(), 2);
    EXPECT_EQ(keyValues[0].kv.int_key(), 1);
    EXPECT_EQ(keyValues[0].kv.int_value(), 100);
    EXPECT_EQ(keyValues[1].kv.int_key(), 2);
    EXPECT_EQ(keyValues[1].kv.int_value(), 200);

    EXPECT_EQ(readPage.getNextLeafOffset(), 0);

    // Clean up
    pageManager.close();
    std::filesystem::remove(fileName);
    std::filesystem::remove_all("test_db");
}

// Test getEOFOffset method
TEST(PageManagerTest, GetEOFOffset) {
    // Ensure test directory exists
    std::filesystem::create_directories("test_db");

    std::string fileName = "test_db/test_page_manager.dat";

    // Remove file if it exists
    if (std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

    PageManager pageManager(fileName);

    size_t pageSize = 4096;

    // Initially, EOF offset should be 0
    EXPECT_EQ(pageManager.getEOFOffset(), pageSize);

    // Allocate a page
    uint64_t offset = pageManager.allocatePage();

    // After allocating, EOF offset should be pageSize
    EXPECT_EQ(pageManager.getEOFOffset(), 2 * pageSize);

    // Allocate another page
    pageManager.allocatePage();
    EXPECT_EQ(pageManager.getEOFOffset(), 3 * pageSize);

    // Clean up
    pageManager.close();
    std::filesystem::remove(fileName);
    std::filesystem::remove_all("test_db");
}

// Test exception handling when reading from an invalid offset
TEST(PageManagerTest, ReadInvalidOffsetThrowsException) {
    // Ensure test directory exists
    std::filesystem::create_directories("test_db");

    std::string fileName = "test_db/test_page_manager.dat";

    // Remove file if it exists
    if (std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

    PageManager pageManager(fileName);

    // Attempt to read from an invalid offset
    uint64_t invalidOffset = 123456; // An offset beyond EOF

    EXPECT_THROW({
        Page page = pageManager.readPage(invalidOffset);
    }, std::runtime_error);

    // Clean up
    pageManager.close();
    std::filesystem::remove(fileName);
    std::filesystem::remove_all("test_db");
}

// // Test closing and reopening the PageManager
// TEST(PageManagerTest, CloseAndReopenPageManager) {
//     // Ensure test directory exists
//     std::filesystem::create_directories("test_db");
//
//     std::string fileName = "test_db/test_page_manager.dat";
//
//     // Remove file if it exists
//     if (std::filesystem::exists(fileName)) {
//         std::filesystem::remove(fileName);
//     }
//
//     {
//         // Scope to close pageManager
//         PageManager pageManager(fileName);
//
//         // Allocate a page
//         uint64_t offset = pageManager.allocatePage();
//
//         // Create and write a page
//         Page page(Page::PageType::LEAF_NODE);
//         KeyValueWrapper kv(1, 100);
//         page.addLeafEntry(kv);
//         pageManager.writePage(offset, page);
//
//         // Close the pageManager
//         pageManager.close();
//     }
//
//     // Reopen the pageManager
//     PageManager pageManager(fileName);
//
//     // Read the page back
//     uint64_t offset = 0; // First page
//     Page readPage = pageManager.readPage(offset);
//
//     // Verify the page content
//     EXPECT_EQ(readPage.getPageType(), Page::PageType::INTERNAL_NODE);
//
//     const auto& keyValues = readPage.getInternalKeys();
//     ASSERT_EQ(keyValues.size(), 1);
//     EXPECT_EQ(keyValues[0].kv.int_key(), 1);
//     EXPECT_EQ(keyValues[0].kv.int_value(), 100);
//
//     // Clean up
//     pageManager.close();
//     std::filesystem::remove(fileName);
//     std::filesystem::remove_all("test_db");
// }

// Test writing multiple pages and reading them back
TEST(PageManagerTest, WriteMultiplePagesAndReadBack) {
    // Ensure test directory exists
    std::filesystem::create_directories("test_db");

    std::string fileName = "test_db/test_page_manager.dat";

    // Remove file if it exists
    if (std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

    PageManager pageManager(fileName);

    size_t pageSize = 4096;

    // Allocate and write multiple pages
    std::vector<uint64_t> offsets;
    for (int i = 0; i < 5; ++i) {
        uint64_t offset = pageManager.allocatePage();
        offsets.push_back(offset);

        Page page(Page::PageType::LEAF_NODE);
        KeyValueWrapper kv(i, i * 10);
        page.addLeafEntry(kv);
        pageManager.writePage(offset, page);
    }

    // Read the pages back and verify content
    for (int i = 0; i < 5; ++i) {
        Page page = pageManager.readPage(offsets[i]);
        EXPECT_EQ(page.getPageType(), Page::PageType::LEAF_NODE);

        const auto& keyValues = page.getLeafEntries();
        ASSERT_EQ(keyValues.size(), 1);
        EXPECT_EQ(keyValues[0].kv.int_key(), i);
        EXPECT_EQ(keyValues[0].kv.int_value(), i * 10);
    }

    // Clean up
    pageManager.close();
    std::filesystem::remove(fileName);
    std::filesystem::remove_all("test_db");
}

// Test writing and reading an internal node page
TEST(PageManagerTest, WriteAndReadInternalNodePage) {
    // Ensure test directory exists
    std::filesystem::create_directories("test_db");

    std::string fileName = "test_db/test_page_manager.dat";

    // Remove file if it exists
    if (std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

    PageManager pageManager(fileName);

    // Allocate a page
    uint64_t offset = pageManager.allocatePage();

    // Create an internal node page
    Page page(Page::PageType::INTERNAL_NODE);
    page.addChildOffset(1000); // C0
    page.addKey(KeyValueWrapper(1, 100)); // K1
    page.addChildOffset(2000); // C1
    page.addKey(KeyValueWrapper(2, 200)); // K2
    page.addChildOffset(3000); // C2

    // Write the page
    pageManager.writePage(offset, page);

    // Read the page back
    Page readPage = pageManager.readPage(offset);

    // Verify the page content
    EXPECT_EQ(readPage.getPageType(), Page::PageType::INTERNAL_NODE);

    const auto& keys = readPage.getInternalKeys();
    const auto& childOffsets = readPage.getChildOffsets();

    ASSERT_EQ(keys.size(), 2);
    ASSERT_EQ(childOffsets.size(), 3);

    EXPECT_EQ(childOffsets[0], 1000);
    EXPECT_EQ(keys[0].kv.int_key(), 1);
    EXPECT_EQ(childOffsets[1], 2000);
    EXPECT_EQ(keys[1].kv.int_key(), 2);
    EXPECT_EQ(childOffsets[2], 3000);

    // Clean up
    pageManager.close();
    std::filesystem::remove(fileName);
    std::filesystem::remove_all("test_db");
}

// Test writing and reading an SST metadata page
TEST(PageManagerTest, WriteAndReadSSTMetadataPage) {
    // Ensure test directory exists
    std::filesystem::create_directories("test_db");

    std::string fileName = "test_db/test_page_manager.dat";

    // Remove file if it exists
    if (std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

    PageManager pageManager(fileName);

    // Allocate a page
    uint64_t offset = pageManager.allocatePage();

    // Create an SST metadata page
    Page page(Page::PageType::SST_METADATA);
    uint64_t rootOffset = 100;
    uint64_t leafBegin = 200;
    uint64_t leafEnd = 300;
    std::string sstFileName = "sst_1.sst";
    page.setMetadata(rootOffset, leafBegin, leafEnd, sstFileName);

    // Write the page
    pageManager.writePage(offset, page);

    // Read the page back
    Page readPage = pageManager.readPage(offset);

    // Verify the page content
    EXPECT_EQ(readPage.getPageType(), Page::PageType::SST_METADATA);

    uint64_t rootOffsetRead, leafBeginRead, leafEndRead;
    std::string sstFileNameRead;
    readPage.getMetadata(rootOffsetRead, leafBeginRead, leafEndRead, sstFileNameRead);

    EXPECT_EQ(rootOffsetRead, rootOffset);
    EXPECT_EQ(leafBeginRead, leafBegin);
    EXPECT_EQ(leafEndRead, leafEnd);
    EXPECT_EQ(sstFileNameRead, sstFileName);

    // Clean up
    pageManager.close();
    std::filesystem::remove(fileName);
    std::filesystem::remove_all("test_db");
}

// Test writing a page with incorrect page size (should throw exception)
TEST(PageManagerTest, WritePageWithIncorrectPageSizeThrowsException) {
    // Ensure test directory exists
    std::filesystem::create_directories("test_db");

    std::string fileName = "test_db/test_page_manager.dat";

    // Remove file if it exists
    if (std::filesystem::exists(fileName)) {
        std::filesystem::remove(fileName);
    }

    PageManager pageManager(fileName, 4096);

    // Allocate a page
    uint64_t offset = pageManager.allocatePage();

    // Create a page with incorrect serialization size
    Page page(Page::PageType::LEAF_NODE);
    KeyValueWrapper kv(1, std::string(5000, 'A')); // Large value to increase page size
    page.addLeafEntry(kv);
    page.setNextLeafOffset(0);

    // Attempt to write the page
    EXPECT_THROW({
        pageManager.writePage(offset, page);
    }, std::runtime_error);

    // Clean up
    pageManager.close();
    std::filesystem::remove(fileName);
    std::filesystem::remove_all("test_db");
}
