//
// Created by Damian Li on 2024-08-28.
//
#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <filesystem>
#include "Memtable.h"
#include "SstFileManager.h"
#include <regex>
#include <thread>

namespace fs = std::filesystem;


/*
 * Unit Tests for:
 * void Memtable::put(const KeyValueWrapper&)
 * Memtable::get(KeyValueWrapper&) -> return KeyValueWrapper
 */
TEST(MemtableTest, BasicInsertAndGet) {
    auto sstFileManager = std::make_shared<SSTFileManager>("defaultDB");
    auto memtable = new Memtable(3, sstFileManager);  // Set a small threshold for testing

    // Insert a KeyValueWrapper pair (int key, int value)
    KeyValueWrapper kv1(10, 100);
    memtable->put(kv1);

    // Retrieve the value using the key
    KeyValueWrapper searchKey(10, "");
    KeyValueWrapper result = memtable->get(searchKey);
    EXPECT_EQ(result.kv.int_value(), 100);  // Verify that the value is correct

    delete memtable;
    fs::remove_all("defaultDB");  // Clean up the created SST files
}

TEST(MemtableTest, InsertMultipleKeyValuePairs) {
    auto sstFileManager = std::make_shared<SSTFileManager>("defaultDB");
    auto memtable = new Memtable(3, sstFileManager);  // Set a small threshold for testing

    // Insert multiple KeyValueWrapper pairs
    memtable->put(KeyValueWrapper(10, 100));
    memtable->put(KeyValueWrapper(20, 200));
    memtable->put(KeyValueWrapper(30, 300));

    // Retrieve the values using the keys
    EXPECT_EQ(memtable->get(KeyValueWrapper(10, "")).kv.int_value(), 100);
    EXPECT_EQ(memtable->get(KeyValueWrapper(20, "")).kv.int_value(), 200);
    EXPECT_EQ(memtable->get(KeyValueWrapper(30, "")).kv.int_value(), 300);

    delete memtable;
    fs::remove_all("defaultDB");  // Clean up the created SST files
}

TEST(MemtableTest, GetValueNotPresent) {
    auto sstFileManager = std::make_shared<SSTFileManager>("defaultDB");
    auto memtable = new Memtable(3, sstFileManager);

    // Insert a KeyValueWrapper pair
    memtable->put(KeyValueWrapper(10, 100));

    // Attempt to retrieve a non-existent key
    KeyValueWrapper result = memtable->get(KeyValueWrapper(20, ""));
    EXPECT_TRUE(result.isEmpty());  // Check if the returned KeyValueWrapper is empty

    delete memtable;
    fs::remove_all("defaultDB");  // Clean up the created SST files
}

TEST(MemtableTest, InsertBeyondThreshold) {
    auto sstFileManager = std::make_shared<SSTFileManager>("defaultDB");
    auto memtable = new Memtable(2, sstFileManager);  // Set a threshold of 2

    // Insert two KeyValueWrapper pairs (should not trigger a flush)
    memtable->put(KeyValueWrapper(10, 100));
    memtable->put(KeyValueWrapper(20, 200));
    EXPECT_EQ(memtable->get(KeyValueWrapper(10, "")).kv.int_value(), 100);
    EXPECT_EQ(memtable->get(KeyValueWrapper(20, "")).kv.int_value(), 200);

    // Insert one more KeyValueWrapper pair (should trigger a flush)
    memtable->put(KeyValueWrapper(30, 300));
    EXPECT_EQ(memtable->get(KeyValueWrapper(30, "")).kv.int_value(), 300);

    delete memtable;
    fs::remove_all("defaultDB");
}

TEST(MemtableTest, ResetCurrentSizeAfterFlush) {
    auto sstFileManager = std::make_shared<SSTFileManager>("defaultDB");
    auto memtable = new Memtable(2, sstFileManager);

    // Insert three KeyValueWrapper pairs (should trigger a flush after the second)
    memtable->put(KeyValueWrapper(10, 100));
    memtable->put(KeyValueWrapper(20, 200));
    memtable->put(KeyValueWrapper(30, 300));  // This should trigger a flush

    // Insert another KeyValueWrapper pair (should be stored after flush)
    memtable->put(KeyValueWrapper(40, 400));
    EXPECT_EQ(memtable->get(KeyValueWrapper(40, "")).kv.int_value(), 400);

    delete memtable;
    fs::remove_all("defaultDB");
}

/*
 * Multiple Flushes with Threshold
 * This test checks that multiple flushes can occur as the threshold is reached multiple times.
 */
TEST(MemtableTest, MultipleFlushesWithThreshold) {
    auto sstFileManager = std::make_shared<SSTFileManager>("defaultDB");
    auto memtable = new Memtable(2, sstFileManager);

    // First flush
    memtable->put(KeyValueWrapper(10, 100));
    memtable->put(KeyValueWrapper(20, 200));
    memtable->put(KeyValueWrapper(30, 300));  // Trigger flush

    // Second flush
    memtable->put(KeyValueWrapper(40, 400));
    memtable->put(KeyValueWrapper(50, 500));
    memtable->put(KeyValueWrapper(60, 600));  // Trigger flush again

    // Retrieve the most recent entries
    EXPECT_EQ(memtable->get(KeyValueWrapper(50, "")).kv.int_value(), 500);
    EXPECT_EQ(memtable->get(KeyValueWrapper(60, "")).kv.int_value(), 600);

    delete memtable;
    fs::remove_all("defaultDB");
}

/*
 * Unit Tests for Scan method in Memtable
 */
TEST(MemtableTest, ScanRange) {
    auto sstFileManager = std::make_shared<SSTFileManager>("defaultDB");
    auto memtable = new Memtable(10, sstFileManager);

    // Insert some KeyValueWrapper pairs
    memtable->put(KeyValueWrapper(10, 100));
    memtable->put(KeyValueWrapper(20, 200));
    memtable->put(KeyValueWrapper(30, 300));
    memtable->put(KeyValueWrapper(40, 400));
    memtable->put(KeyValueWrapper(50, 500));

    // Scan a range from 20 to 40
    std::set<KeyValueWrapper> results;
    memtable->scan(KeyValueWrapper(20, ""), KeyValueWrapper(40, ""), results);

    // Verify the results
    EXPECT_EQ(results.size(), 3);
    EXPECT_EQ(results.begin()->kv.int_value(), 200);
    EXPECT_EQ(std::next(results.begin(), 1)->kv.int_value(), 300);
    EXPECT_EQ(std::next(results.begin(), 2)->kv.int_value(), 400);

    delete memtable;
    fs::remove_all("defaultDB");
}
