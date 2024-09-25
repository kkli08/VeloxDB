//
// Created by damian on 9/24/24.
//
//
// DiskBTreeTest.cpp
//

#include <gtest/gtest.h>
#include "DiskBTree.h"
#include "KeyValue.h"
#include <filesystem>
#include <cstdlib>
#include <ctime>

namespace fs = std::filesystem;

// Helper function to generate a vector of KeyValueWrapper with integer keys and values
std::vector<KeyValueWrapper> generateIntKeyValues(size_t count) {
    std::vector<KeyValueWrapper> keyValues;
    keyValues.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        keyValues.emplace_back(static_cast<int>(i), static_cast<int>(i*10));
    }
    return keyValues;
}

// Helper function to generate a vector of KeyValueWrapper with random integer keys and values
std::vector<KeyValueWrapper> generateRandomIntKeyValues(size_t count) {
    std::vector<KeyValueWrapper> keyValues;
    keyValues.reserve(count);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (size_t i = 0; i < count; ++i) {
        int key = std::rand();
        int value = std::rand();
        keyValues.emplace_back(key, value);
    }
    return keyValues;
}

// Clean up function to remove test files and directories
void cleanUp(const std::string& fileName) {
    if (fs::exists(fileName)) {
        fs::remove(fileName);
    }
}

// Test the constructor that builds a new B-tree from memtable data
TEST(DiskBTreeTest, ConstructorBuildsTree) {
    std::string sstFileName = "test_sst_build.sst";
    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(100);

    // Create DiskBTree
    DiskBTree btree(sstFileName, 3, keyValues);

    // Check that the file was created
    EXPECT_TRUE(fs::exists(sstFileName));

    // Clean up
    cleanUp(sstFileName);
}

// Test the constructor that opens an existing SST file
TEST(DiskBTreeTest, ConstructorOpensExistingTree) {
    std::string sstFileName = "test_sst_existing.sst";
    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(100);

    // Create DiskBTree and build tree
    {
        DiskBTree btree(sstFileName, 3, keyValues);
    }

    // Open existing DiskBTree
    DiskBTree btree(sstFileName, 3);

    // Check that the file exists
    EXPECT_TRUE(fs::exists(sstFileName));

    // Clean up
    cleanUp(sstFileName);
}

// Test getFileName method
TEST(DiskBTreeTest, GetFileName) {
    std::string sstFileName = "test_sst_getfilename.sst";
    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(10);

    // Create DiskBTree
    DiskBTree btree(sstFileName, 3, keyValues);

    // Get file name
    EXPECT_EQ(btree.getFileName(), sstFileName);

    // Clean up
    cleanUp(sstFileName);
}

// Test search method with existing keys
TEST(DiskBTreeTest, SearchLessKeys) {
    std::string sstFileName = "test_sst_search_less_keys.sst";

    // Clean up before and after the test
    auto cleanUp = [](const std::string& fileName) {
        if (std::filesystem::exists(fileName)) {
            std::filesystem::remove(fileName);
        }
    };

    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(10);

    // Create DiskBTree
    DiskBTree btree(sstFileName, 3, keyValues);

    // Search for existing keys
    for (int i = 0; i < 10; ++i) {
        std::cout << "i == " << i << std::endl;
        KeyValueWrapper* result = btree.search(KeyValueWrapper(i, 0));
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->kv.int_key(), i);
        EXPECT_EQ(result->kv.int_value(), i * 10);
        delete result; // Clean up allocated memory
    }

    // Clean up
    cleanUp(sstFileName);
}



// Test search method with existing keys
TEST(DiskBTreeTest, SearchExistingKeys) {
    std::string sstFileName = "test_sst_search_existing.sst";

    // Clean up before and after the test
    auto cleanUp = [](const std::string& fileName) {
        if (std::filesystem::exists(fileName)) {
            std::filesystem::remove(fileName);
        }
    };

    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(1000);

    // Create DiskBTree
    DiskBTree btree(sstFileName, 3, keyValues);

    // Search for existing keys
    for (int i = 0; i < 1000; i += 100) {
        std::cout << "i == " << i << std::endl;
        KeyValueWrapper* result = btree.search(KeyValueWrapper(i, 0));
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->kv.int_key(), i);
        EXPECT_EQ(result->kv.int_value(), i * 10);
        delete result; // Clean up allocated memory
    }

    // Clean up
    cleanUp(sstFileName);
}


// Test search method with non-existing keys
TEST(DiskBTreeTest, SearchNonExistingKeys) {
    std::string sstFileName = "test_sst_search_nonexisting.sst";
    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(1000);

    // Create DiskBTree
    DiskBTree btree(sstFileName, 3, keyValues);

    // Search for non-existing keys
    for (int i = 1001; i < 1100; ++i) {
        KeyValueWrapper* result = btree.search(KeyValueWrapper(i, 0));
        EXPECT_EQ(result, nullptr);
    }

    // Clean up
    cleanUp(sstFileName);
}

// Test search method with large dataset
TEST(DiskBTreeTest, SearchLargeDataset) {
    std::string sstFileName = "test_sst_search_large.sst";
    cleanUp(sstFileName);

    // Generate large test data
    size_t dataSize = 100000; // 1e5 entries
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(dataSize);

    // Create DiskBTree
    DiskBTree btree(sstFileName, 50, keyValues);

    // Search for existing keys
    for (size_t i = 0; i < dataSize; i += 10000) {
        KeyValueWrapper* result = btree.search(KeyValueWrapper(static_cast<int>(i), 0));
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->kv.int_key(), static_cast<int>(i));
        EXPECT_EQ(result->kv.int_value(), static_cast<int>(i * 10));
    }

    // Search for non-existing key
    KeyValueWrapper* result = btree.search(KeyValueWrapper(-1, 0));
    EXPECT_EQ(result, nullptr);

    // Clean up
    cleanUp(sstFileName);
}

// Test scan method with small range
TEST(DiskBTreeTest, ScanSmallRange) {
    std::string sstFileName = "test_sst_scan_small.sst";
    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(100);

    // Create DiskBTree
    DiskBTree btree(sstFileName, 3, keyValues);

    // Scan a range
    std::vector<KeyValueWrapper> result;
    btree.scan(KeyValueWrapper(20, 0), KeyValueWrapper(30, 0), result);

    // Verify the results
    EXPECT_EQ(result.size(), 11);
    for (int i = 20; i <= 30; ++i) {
        EXPECT_EQ(result[i - 20].kv.int_key(), i);
        EXPECT_EQ(result[i - 20].kv.int_value(), i * 10);
    }

    // Clean up
    cleanUp(sstFileName);
}

// Test scan method with full range
TEST(DiskBTreeTest, ScanFullRange) {
    std::string sstFileName = "test_sst_scan_full.sst";
    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(1000);

    // Create DiskBTree
    DiskBTree btree(sstFileName, 3, keyValues);

    // Scan the full range
    std::vector<KeyValueWrapper> result;
    btree.scan(KeyValueWrapper(0, 0), KeyValueWrapper(999, 0), result);

    // Verify the results
    EXPECT_EQ(result.size(), 1000);
    for (int i = 0; i < 1000; ++i) {
        EXPECT_EQ(result[i].kv.int_key(), i);
        EXPECT_EQ(result[i].kv.int_value(), i * 10);
    }

    // Clean up
    cleanUp(sstFileName);
}

// Test scan method with large dataset
TEST(DiskBTreeTest, ScanLargeDataset) {
    std::string sstFileName = "test_sst_scan_large.sst";
    cleanUp(sstFileName);

    // Generate large test data
    size_t dataSize = 100000; // 1e5 entries
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(dataSize);

    // Create DiskBTree
    DiskBTree btree(sstFileName, 50, keyValues);

    // Scan a range
    std::vector<KeyValueWrapper> result;
    btree.scan(KeyValueWrapper(50000, 0), KeyValueWrapper(50010, 0), result);

    // Verify the results
    EXPECT_EQ(result.size(), 11);
    for (size_t i = 0; i <= 10; ++i) {
        EXPECT_EQ(result[i].kv.int_key(), static_cast<int>(50000 + i));
        EXPECT_EQ(result[i].kv.int_value(), static_cast<int>((50000 + i) * 10));
    }

    // Clean up
    cleanUp(sstFileName);
}

// Test scan method with no results
TEST(DiskBTreeTest, ScanNoResults) {
    std::string sstFileName = "test_sst_scan_no_results.sst";
    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(1000);

    // Create DiskBTree
    DiskBTree btree(sstFileName, 3, keyValues);

    // Scan a range with no results
    std::vector<KeyValueWrapper> result;
    btree.scan(KeyValueWrapper(2000, 0), KeyValueWrapper(3000, 0), result);

    // Verify the results
    EXPECT_EQ(result.size(), 0);

    // Clean up
    cleanUp(sstFileName);
}

// Test destructor (implicit test by ensuring no exceptions or errors occur during destruction)
TEST(DiskBTreeTest, DestructorTest) {
    std::string sstFileName = "test_sst_destructor.sst";
    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(1000);

    // Create and destroy DiskBTree
    {
        DiskBTree btree(sstFileName, 3, keyValues);
    }

    // Reopen and destroy
    {
        DiskBTree btree(sstFileName, 3);
    }

    // If no exceptions occurred, the destructor works correctly
    SUCCEED();

    // Clean up
    cleanUp(sstFileName);
}

// Test constructor with invalid degree (edge case)
TEST(DiskBTreeTest, ConstructorInvalidDegree) {
    std::string sstFileName = "test_sst_invalid_degree.sst";
    cleanUp(sstFileName);

    // Generate test data
    std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(100);

    // Attempt to create DiskBTree with invalid degree
    EXPECT_THROW({
        DiskBTree btree(sstFileName, 0, keyValues);
    }, std::invalid_argument);

    // Clean up
    cleanUp(sstFileName);
}

// Test search method with string keys
TEST(DiskBTreeTest, SearchStringKeys) {
    std::string sstFileName = "test_sst_search_string_keys.sst";
    cleanUp(sstFileName);

    // Generate test data with string keys
    std::vector<KeyValueWrapper> keyValues = {
        KeyValueWrapper("apple", "fruit"),
        KeyValueWrapper("carrot", "vegetable"),
        KeyValueWrapper("banana", "fruit"),
        KeyValueWrapper("tomato", "vegetable"),
        KeyValueWrapper("grape", "fruit")
    };

    // Sort the keyValues
    std::sort(keyValues.begin(), keyValues.end());

    // Create DiskBTree
    DiskBTree btree(sstFileName, 3, keyValues);

    // Search for existing key
    KeyValueWrapper* result = btree.search(KeyValueWrapper("banana", ""));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->kv.string_value(), "fruit");

    // Search for non-existing key
    result = btree.search(KeyValueWrapper("orange", ""));
    EXPECT_EQ(result, nullptr);

    // Clean up
    cleanUp(sstFileName);
}

// Test scan method with string keys
TEST(DiskBTreeTest, ScanStringKeys) {
    std::string sstFileName = "test_sst_scan_string_keys.sst";
    cleanUp(sstFileName);

    // Generate test data with string keys
    std::vector<KeyValueWrapper> keyValues = {
        KeyValueWrapper("apple", "fruit"),
        KeyValueWrapper("banana", "fruit"),
        KeyValueWrapper("carrot", "vegetable"),
        KeyValueWrapper("grape", "fruit"),
        KeyValueWrapper("tomato", "vegetable")
    };

    // Sort the keyValues
    std::sort(keyValues.begin(), keyValues.end());

    // Create DiskBTree
    DiskBTree btree(sstFileName, 3, keyValues);

    // Scan a range
    std::vector<KeyValueWrapper> result;
    btree.scan(KeyValueWrapper("banana", ""), KeyValueWrapper("grape", ""), result);

    // Verify the results
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result[0].kv.string_value(), "fruit");
    EXPECT_EQ(result[1].kv.string_value(), "vegetable");
    EXPECT_EQ(result[2].kv.string_value(), "fruit");

    // Clean up
    cleanUp(sstFileName);
}

// // Test multiple searches in a loop to assess performance
// TEST(DiskBTreeTest, SearchPerformanceTest) {
//     std::string sstFileName = "test_sst_search_performance.sst";
//     cleanUp(sstFileName);
//
//     // Generate large test data
//     size_t dataSize = 1000000; // 1e6 entries
//     std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(dataSize);
//
//     // Create DiskBTree
//     DiskBTree btree(sstFileName, 50, keyValues);
//
//     // Perform multiple searches
//     for (size_t i = 0; i < dataSize; i += 100000) {
//         KeyValueWrapper* result = btree.search(KeyValueWrapper(static_cast<int>(i), 0));
//         ASSERT_NE(result, nullptr);
//         EXPECT_EQ(result->kv.int_key(), static_cast<int>(i));
//         EXPECT_EQ(result->kv.int_value(), static_cast<int>(i * 10));
//     }
//
//     // Clean up
//     cleanUp(sstFileName);
// }

// // Test scan method over the entire dataset to assess performance
// TEST(DiskBTreeTest, ScanPerformanceTest) {
//     std::string sstFileName = "test_sst_scan_performance.sst";
//     cleanUp(sstFileName);
//
//     // Generate large test data
//     size_t dataSize = 1000000; // 1e6 entries
//     std::vector<KeyValueWrapper> keyValues = generateIntKeyValues(dataSize);
//
//     // Create DiskBTree
//     DiskBTree btree(sstFileName, 50, keyValues);
//
//     // Scan the entire range
//     std::vector<KeyValueWrapper> result;
//     btree.scan(KeyValueWrapper(0, 0), KeyValueWrapper(static_cast<int>(dataSize - 1), 0), result);
//
//     // Verify the results
//     EXPECT_EQ(result.size(), dataSize);
//     EXPECT_EQ(result.front().kv.int_key(), 0);
//     EXPECT_EQ(result.back().kv.int_key(), static_cast<int>(dataSize - 1));
//
//     // Clean up
//     cleanUp(sstFileName);
// }
