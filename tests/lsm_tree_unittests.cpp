//
// Created by Damian Li on 2024-11-03.
//

#include <gtest/gtest.h>
#include "LSMTree.h"
#include "KeyValue.h"
#include <filesystem>
#include <cstdlib>
#include <ctime>

namespace fs = std::filesystem;

// Helper function to generate a vector of KeyValueWrapper with integer keys and values
std::vector<KeyValueWrapper> lsm_generateIntKeyValues(size_t count) {
    std::vector<KeyValueWrapper> keyValues;
    keyValues.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        keyValues.emplace_back(static_cast<int>(i), static_cast<int>(i * 10));
    }
    return keyValues;
}

// Helper function to generate a vector of KeyValueWrapper with random integer keys and values
std::vector<KeyValueWrapper> lsm_generateRandomIntKeyValues(size_t count) {
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

// Clean up function to remove test directories
void cleanUpDir(const std::string& dirName) {
    if (fs::exists(dirName)) {
        fs::remove_all(dirName);
    }
}

// Test 1: Constructor initializes empty LSMTree
TEST(LSMTreeTest, ConstructorInitializesEmptyTree) {
    std::string dbPath = "test_lsm_constructor_empty";
    cleanUpDir(dbPath);

    // Create LSMTree
    LSMTree lsmTree(1000, dbPath);

    // Verify that the levels are empty (only memtable)
    EXPECT_EQ(lsmTree.getNumLevels(), 1); // Level 0: memtable

    // Clean up
    cleanUpDir(dbPath);
}

// Test 2: Put and Get single key in Memtable
TEST(LSMTreeTest, PutAndGetSingleKey) {
    std::string dbPath = "test_lsm_put_get_single";
    cleanUpDir(dbPath);

    // Create LSMTree
    LSMTree lsmTree(1000, dbPath);

    // Insert a key
    KeyValueWrapper kv(1, 100);
    lsmTree.put(kv);

    // Get the key
    KeyValueWrapper result = lsmTree.get(kv);

    // Verify
    EXPECT_EQ(result.kv.int_key(), 1);
    EXPECT_EQ(result.kv.int_value(), 100);

    // Clean up
    cleanUpDir(dbPath);
}

// Test 3: Put multiple keys and flush Memtable to Level 1
TEST(LSMTreeTest, PutMultipleKeysFlushMemtable) {
    std::string dbPath = "test_lsm_put_flush";
    cleanUpDir(dbPath);

    size_t memtableSize = 10; // Small size to trigger flush quickly
    LSMTree lsmTree(memtableSize, dbPath);

    // Insert 15 keys to trigger flush
    std::vector<KeyValueWrapper> keyValues = lsm_generateIntKeyValues(15);
    for (const auto& kv : keyValues) {
        lsmTree.put(kv);
    }

    // After flush, memtable should have 5, Level 1 should have 10
    // Since LSMTree doesn't expose levels, verify via 'get'
    for (size_t i = 0; i < 15; ++i) {
        KeyValueWrapper queryKv(static_cast<int>(i), 0);
        KeyValueWrapper result = lsmTree.get(queryKv);
        EXPECT_EQ(result.kv.int_key(), static_cast<int>(i));
        EXPECT_EQ(result.kv.int_value(), static_cast<int>(i * 10));
    }

    // Verify number of levels
    EXPECT_EQ(lsmTree.getNumLevels(), 2); // Memtable + Level 1

    // Clean up
    cleanUpDir(dbPath);
}

// Test 4: Get keys from Level 1 after flush
TEST(LSMTreeTest, GetKeysAfterFlush) {
    std::string dbPath = "test_lsm_get_after_flush";
    cleanUpDir(dbPath);

    size_t memtableSize = 10; // Small size to trigger flush quickly
    LSMTree lsmTree(memtableSize, dbPath);

    // Insert 15 keys to trigger flush
    std::vector<KeyValueWrapper> keyValues = lsm_generateIntKeyValues(15);
    for (const auto& kv : keyValues) {
        lsmTree.put(kv);
    }

    // The first 10 keys should be in Level 1
    for (size_t i = 0; i < 10; ++i) {
        KeyValueWrapper queryKv(static_cast<int>(i), 0);
        KeyValueWrapper result = lsmTree.get(queryKv);
        EXPECT_EQ(result.kv.int_key(), static_cast<int>(i));
        EXPECT_EQ(result.kv.int_value(), static_cast<int>(i * 10));
    }

    // The last 5 keys should be in memtable
    for (size_t i = 10; i < 15; ++i) {
        KeyValueWrapper queryKv(static_cast<int>(i), 0);
        KeyValueWrapper result = lsmTree.get(queryKv);
        EXPECT_EQ(result.kv.int_key(), static_cast<int>(i));
        EXPECT_EQ(result.kv.int_value(), static_cast<int>(i * 10));
    }

    // Verify number of levels
    EXPECT_EQ(lsmTree.getNumLevels(), 2); // Memtable + Level1

    // Clean up
    cleanUpDir(dbPath);
}

// Test 5: Insert keys to trigger a merge to Level 2, verify Level 2 has merged SSTable
TEST(LSMTreeTest, TriggerMergeAndVerifyLevels) {
    std::string dbPath = "test_lsm_trigger_merge";
    cleanUpDir(dbPath);

    size_t memtableSize = 5; // Small size to trigger multiple flushes and merge
    LSMTree lsmTree(memtableSize, dbPath);

    // Insert 16 keys to trigger flush and merge
    std::vector<KeyValueWrapper> keyValues = lsm_generateIntKeyValues(16);
    for (const auto& kv : keyValues) {
        lsmTree.put(kv);
    }


    // Verify number of levels
    EXPECT_EQ(lsmTree.getNumLevels(), 3); // Memtable + Level1 + Level2

    // Clean up
    cleanUpDir(dbPath);
}

// Test 6: Get keys after merge
TEST(LSMTreeTest, GetKeysAfterMerge) {
    std::string dbPath = "test_lsm_get_after_merge";
    cleanUpDir(dbPath);

    size_t memtableSize = 5; // Small size to trigger multiple flushes and merge
    LSMTree lsmTree(memtableSize, dbPath);

    // Insert 16 keys to trigger flush and merge
    std::vector<KeyValueWrapper> keyValues = lsm_generateIntKeyValues(16);
    for (const auto& kv : keyValues) {
        lsmTree.put(kv);
    }

    // Retrieve all keys
    for (size_t i = 1; i < 16; ++i) {
        cout << "i = " << i << endl;
        KeyValueWrapper queryKv(static_cast<int>(i), 0);
        KeyValueWrapper result = lsmTree.get(queryKv);

        EXPECT_EQ(result.kv.int_key(), static_cast<int>(i));
        // EXPECT_EQ(result.kv.int_value(), static_cast<int>(i * 10));
    }

    // Verify number of levels
    EXPECT_EQ(lsmTree.getNumLevels(), 3); // Memtable + Level1 + Level2

    // Clean up
    cleanUpDir(dbPath);
}

// Test 7: Scan across memtable and SSTables
TEST(LSMTreeTest, ScanAcrossLevels) {
    std::string dbPath = "test_lsm_scan_across";
    cleanUpDir(dbPath);

    size_t memtableSize = 5; // Small size to trigger flushes
    LSMTree lsmTree(memtableSize, dbPath);

    // Insert 15 keys to populate Level1 and some in memtable
    std::vector<KeyValueWrapper> keyValues = lsm_generateIntKeyValues(19);
    for (const auto& kv : keyValues) {
        lsmTree.put(kv);
    }

    // Define scan range
    KeyValueWrapper startKey(5, 0);
    KeyValueWrapper endKey(12, 0);

    // Perform scan
    std::vector<KeyValueWrapper> scanResult;
    lsmTree.scan(startKey, endKey, scanResult);

    // Expected keys: 5 to 12 inclusive
    EXPECT_EQ(scanResult.size(), 8);
    for (int i = 5; i <= 12; ++i) {
        EXPECT_EQ(scanResult[i - 5].kv.int_key(), i);
        EXPECT_EQ(scanResult[i - 5].kv.int_value(), i * 10);
    }

    // Clean up
    cleanUpDir(dbPath);
}

// Test 8: Save and Load state, verify data integrity
TEST(LSMTreeTest, SaveAndLoadState) {
    std::string dbPath = "test_lsm_save_load";
    cleanUpDir(dbPath);

    size_t memtableSize = 10;
    {
        // Create and populate LSMTree
        LSMTree lsmTree(memtableSize, dbPath);
        std::vector<KeyValueWrapper> keyValues = lsm_generateIntKeyValues(20);
        for (const auto& kv : keyValues) {
            lsmTree.put(kv);
        }
        // Flush is handled by LSMTree when put triggers flush
        // Save state
        lsmTree.saveState();
    }

    {
        // Load LSMTree
        LSMTree lsmTree(memtableSize, dbPath);
        lsmTree.loadState();

        // Verify data
        for (size_t i = 0; i < 20; ++i) {
            KeyValueWrapper queryKv(static_cast<int>(i), 0);
            KeyValueWrapper result = lsmTree.get(queryKv);
            EXPECT_EQ(result.kv.int_key(), static_cast<int>(i));
            // EXPECT_EQ(result.kv.int_value(), static_cast<int>(i * 10));
        }
    }

    // Clean up
    cleanUpDir(dbPath);
}

