//
// Created by damian on 9/25/24.
//
#include <gtest/gtest.h>
#include "DiskBTree.h"
#include "KeyValue.h"
#include "SstFileManager.h"
#include <filesystem>
#include <cstdlib>
#include <ctime>

// Helper function to generate sequential integer key-values
std::vector<KeyValueWrapper> generateSequentialKeyValues(size_t start, size_t count) {
    std::vector<KeyValueWrapper> keyValues;
    keyValues.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        keyValues.emplace_back(static_cast<int>(start + i), static_cast<int>((start + i) * 10));
    }
    return keyValues;
}

// Helper function to generate random integer key-values
std::vector<KeyValueWrapper> generateRandomKeyValues(size_t count, int keyRangeStart = 0, int keyRangeEnd = 1000000) {
    std::vector<KeyValueWrapper> keyValues;
    keyValues.reserve(count);
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    for (size_t i = 0; i < count; ++i) {
        int key = keyRangeStart + std::rand() % (keyRangeEnd - keyRangeStart);
        int value = std::rand();
        keyValues.emplace_back(key, value);
    }
    return keyValues;
}

// Helper function to clean up the test directory
void cleanUpDirectory(const std::string& dirPath) {
    if (std::filesystem::exists(dirPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            std::filesystem::remove_all(entry.path());
        }
    }
}


/*
 * unit tests for flushMemtable
 */

TEST(SSTFileManagerTest, FlushSingleMemtable) {
    std::string testDir = "test_sst_files_single";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Generate test data
    auto keyValues = generateSequentialKeyValues(0, 100);

    // Flush memtable
    manager.flushMemtable(keyValues);

    // Verify that one SST file is created
    int sstFileCount = std::distance(std::filesystem::directory_iterator(testDir), std::filesystem::directory_iterator{});
    EXPECT_EQ(sstFileCount, 1);

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, FlushMultipleMemtables) {
    std::string testDir = "test_sst_files_multiple";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Generate and flush multiple memtables
    for (int i = 0; i < 5; ++i) {
        auto keyValues = generateSequentialKeyValues(i * 100, 100);
        manager.flushMemtable(keyValues);
    }

    // Verify that five SST files are created
    int sstFileCount = std::distance(std::filesystem::directory_iterator(testDir), std::filesystem::directory_iterator{});
    EXPECT_EQ(sstFileCount, 5);

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, FlushEmptyMemtable) {
    std::string testDir = "test_sst_files_empty";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Flush empty memtable
    std::vector<KeyValueWrapper> keyValues;
    manager.flushMemtable(keyValues);

    // Verify that no SST file is created
    int sstFileCount = std::distance(std::filesystem::directory_iterator(testDir), std::filesystem::directory_iterator{});
    EXPECT_EQ(sstFileCount, 0);

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, FlushLargeMemtable) {
    std::string testDir = "test_sst_files_large";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Generate large dataset
    auto keyValues = generateSequentialKeyValues(0, 10000);

    // Flush memtable
    manager.flushMemtable(keyValues);

    // Verify that one SST file is created
    int sstFileCount = std::distance(std::filesystem::directory_iterator(testDir), std::filesystem::directory_iterator{});
    EXPECT_EQ(sstFileCount, 1);

    // Optionally, check that the SST file is of expected size
    // This can be done by checking the file size, but it's not essential for this test

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, FlushMemtableCreatesCorrectSSTFiles) {
    std::string testDir = "test_sst_files_correct";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Generate test data
    auto keyValues = generateSequentialKeyValues(0, 100);

    // Flush memtable
    manager.flushMemtable(keyValues);

    // Verify that the SST file exists with the correct name pattern
    int sstFileCount = 0;
    for (const auto& entry : std::filesystem::directory_iterator(testDir)) {
        if (entry.path().extension() == ".sst") {
            ++sstFileCount;
            EXPECT_TRUE(entry.path().filename().string().find("sst_") != std::string::npos);
        }
    }
    EXPECT_EQ(sstFileCount, 1);

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

/*
 * unit test for search
 */

TEST(SSTFileManagerTest, SearchKeyInMostRecentSST) {
    std::string testDir = "test_sst_search_recent";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Flush two memtables
    auto keyValues1 = generateSequentialKeyValues(0, 100);
    manager.flushMemtable(keyValues1);

    auto keyValues2 = generateSequentialKeyValues(100, 100);
    manager.flushMemtable(keyValues2);

    // Search for keys in the most recent SST
    for (int i = 100; i < 200; i += 10) {
        KeyValueWrapper* result = manager.search(KeyValueWrapper(i, 0));
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->kv.int_key(), i);
        EXPECT_EQ(result->kv.int_value(), i * 10);
        delete result;
    }

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, SearchKeyInOlderSSTs) {
    std::string testDir = "test_sst_search_older";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Flush multiple memtables
    for (int i = 0; i < 5; ++i) {
        auto keyValues = generateSequentialKeyValues(i * 100, 100);
        manager.flushMemtable(keyValues);
    }

    // Search for keys in older SSTs
    for (int i = 0; i < 500; i += 50) {
        KeyValueWrapper* result = manager.search(KeyValueWrapper(i, 0));
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->kv.int_key(), i);
        EXPECT_EQ(result->kv.int_value(), i * 10);
        delete result;
    }

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, SearchNonExistentKey) {
    std::string testDir = "test_sst_search_nonexistent";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Flush memtable
    auto keyValues = generateSequentialKeyValues(0, 100);
    manager.flushMemtable(keyValues);

    // Search for a key that does not exist
    KeyValueWrapper* result = manager.search(KeyValueWrapper(200, 0));
    EXPECT_EQ(result, nullptr);

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, SearchLargeNumberOfSSTs) {
    std::string testDir = "test_sst_search_many_ssts";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Flush many small memtables
    for (int i = 0; i < 50; ++i) {
        auto keyValues = generateSequentialKeyValues(i * 10, 10);
        manager.flushMemtable(keyValues);
    }

    // Search for keys across SSTs
    for (int i = 0; i < 500; i += 25) {
        KeyValueWrapper* result = manager.search(KeyValueWrapper(i, 0));
        if (i % 10 == 0) {
            ASSERT_NE(result, nullptr);
            EXPECT_EQ(result->kv.int_key(), i);
            EXPECT_EQ(result->kv.int_value(), i * 10);
            delete result;
        } else {
            EXPECT_EQ(result, nullptr);
        }
    }

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, SearchWithLargeDataset) {
    std::string testDir = "test_sst_search_large_dataset";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Generate and flush large datasets
    for (int i = 0; i < 5; ++i) {
        auto keyValues = generateSequentialKeyValues(i * 10000, 10000);
        manager.flushMemtable(keyValues);
    }

    // Search for keys
    for (int i = 0; i < 50000; i += 5000) {
        KeyValueWrapper* result = manager.search(KeyValueWrapper(i, 0));
        ASSERT_NE(result, nullptr);
        EXPECT_EQ(result->kv.int_key(), i);
        EXPECT_EQ(result->kv.int_value(), i * 10);
        delete result;
    }

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}


/*
 * Unit tests for scan
 */

TEST(SSTFileManagerTest, ScanRangeAcrossMultipleSSTs) {
    std::string testDir = "test_sst_scan_multiple_ssts";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Flush multiple memtables
    for (int i = 0; i < 5; ++i) {
        auto keyValues = generateSequentialKeyValues(i * 100, 100);
        manager.flushMemtable(keyValues);
    }

    // Scan a range that spans multiple SSTs
    std::vector<KeyValueWrapper> result;
    manager.scan(KeyValueWrapper(150, 0), KeyValueWrapper(350, 0), result);

    // Verify the scan results
    EXPECT_EQ(result.size(), 201); // Keys from 150 to 350 inclusive

    for (size_t i = 0; i < result.size(); ++i) {
        int expectedKey = 150 + static_cast<int>(i);
        EXPECT_EQ(result[i].kv.int_key(), expectedKey);
        EXPECT_EQ(result[i].kv.int_value(), expectedKey * 10);
    }

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, ScanRangeWithinSingleSST) {
    std::string testDir = "test_sst_scan_single_sst";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Flush a single memtable
    auto keyValues = generateSequentialKeyValues(0, 500);
    manager.flushMemtable(keyValues);

    // Scan a range within the single SST
    std::vector<KeyValueWrapper> result;
    manager.scan(KeyValueWrapper(100, 0), KeyValueWrapper(200, 0), result);

    // Verify the scan results
    EXPECT_EQ(result.size(), 101); // Keys from 100 to 200 inclusive

    for (size_t i = 0; i < result.size(); ++i) {
        int expectedKey = 100 + static_cast<int>(i);
        EXPECT_EQ(result[i].kv.int_key(), expectedKey);
        EXPECT_EQ(result[i].kv.int_value(), expectedKey * 10);
    }

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, ScanNoMatchingKeys) {
    std::string testDir = "test_sst_scan_no_match";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Flush memtable
    auto keyValues = generateSequentialKeyValues(0, 100);
    manager.flushMemtable(keyValues);

    // Scan a range with no matching keys
    std::vector<KeyValueWrapper> result;
    manager.scan(KeyValueWrapper(200, 0), KeyValueWrapper(300, 0), result);

    // Verify the scan results
    EXPECT_EQ(result.size(), 0);

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, ScanWithLargeDataset) {
    std::string testDir = "test_sst_scan_large_dataset";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Generate and flush large datasets
    for (int i = 0; i < 10; ++i) {
        auto keyValues = generateSequentialKeyValues(i * 10000, 10000);
        manager.flushMemtable(keyValues);
    }

    // Scan a large range
    std::vector<KeyValueWrapper> result;
    manager.scan(KeyValueWrapper(25000, 0), KeyValueWrapper(75000, 0), result);

    // Verify the scan results
    EXPECT_EQ(result.size(), 50001); // Keys from 25000 to 75000 inclusive

    for (size_t i = 0; i < result.size(); ++i) {
        int expectedKey = 25000 + static_cast<int>(i);
        EXPECT_EQ(result[i].kv.int_key(), expectedKey);
        EXPECT_EQ(result[i].kv.int_value(), expectedKey * 10);
    }

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}

TEST(SSTFileManagerTest, ScanOverlappingKeysAcrossSSTs) {
    std::string testDir = "test_sst_scan_overlapping";
    int degree = 3;

    cleanUpDirectory(testDir);
    std::filesystem::create_directory(testDir);

    SSTFileManager manager(testDir, degree);

    // Flush overlapping memtables
    auto keyValues1 = generateSequentialKeyValues(0, 200);
    manager.flushMemtable(keyValues1);

    auto keyValues2 = generateSequentialKeyValues(100, 200); // Overlaps with previous
    manager.flushMemtable(keyValues2);

    // Scan a range that includes overlapping keys
    std::vector<KeyValueWrapper> result;
    manager.scan(KeyValueWrapper(50, 0), KeyValueWrapper(250, 0), result);

    // Verify the scan results
    // Due to overlapping keys, duplicates should be resolved
    EXPECT_EQ(result.size(), 201); // Keys from 50 to 250 inclusive

    // Verify that the values are from the most recent SST
    for (size_t i = 0; i < result.size(); ++i) {
        int expectedKey = 50 + static_cast<int>(i);
        EXPECT_EQ(result[i].kv.int_key(), expectedKey);
        if (expectedKey >= 100 && expectedKey <= 299) {
            EXPECT_EQ(result[i].kv.int_value(), expectedKey * 10); // From the second memtable
        } else {
            EXPECT_EQ(result[i].kv.int_value(), expectedKey * 10); // From the first memtable
        }
    }

    cleanUpDirectory(testDir);
    std::filesystem::remove(testDir);
}
