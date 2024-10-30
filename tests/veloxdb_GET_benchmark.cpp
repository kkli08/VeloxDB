//
// Created by damian on 9/26/24.
//
#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <iostream>
#include "VeloxDB.h"

// TEST(VeloxDBBenchMarkTest, LargeScaleInsertAndScanRange) {
//     int memtableSize = 10000;  // Adjust memtable size for your performance needs
//     auto db = std::make_unique<VeloxDB>(memtableSize, 3);
//     db->Open("test_db");
//
//     const int numEntries = 1e6;  // Insert 1e6 key-value pairs
//     std::cout << "Starting insert of " << numEntries << " key-value pairs..." << std::endl;
//
//     auto startInsert = std::chrono::high_resolution_clock::now();
//
//     // Insert key-value pairs
//     for (int i = 0; i < numEntries; ++i) {
//         db->Put(i, "value_" + std::to_string(i));
//     }
//
//     auto endInsert = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> insertDuration = endInsert - startInsert;
//
//     std::cout << "Insert completed. Time taken: " << insertDuration.count() << " seconds" << std::endl;
//     std::cout << "Average time per insert: " << (insertDuration.count() / numEntries) * 1e6 << " microseconds" << std::endl;
//
//     // Test Scan performance
//     std::cout << "Starting scan for key-value pairs between 1 and 50000..." << std::endl;
//
//     auto startScan = std::chrono::high_resolution_clock::now();
//
//     std::set<KeyValueWrapper> resultSet = db->Scan(KeyValueWrapper(1, ""), KeyValueWrapper(50000, ""));
//
//     auto endScan = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> scanDuration = endScan - startScan;
//
//     ASSERT_EQ(resultSet.size(), 50000);  // Verify that 50,000 key-value pairs were scanned
//
//     std::cout << "Scan completed. Time taken: " << scanDuration.count() << " seconds" << std::endl;
//     std::cout << "Average time per scan operation: " << (scanDuration.count() / 50000) * 1e6 << " microseconds" << std::endl;
//
//     // Verify the content of the result set
//     auto it = resultSet.begin();
//     for (int i = 1; i <= 50000; ++i, ++it) {
//         EXPECT_EQ(it->kv.int_key(), i);
//         EXPECT_EQ(it->kv.string_value(), "value_" + std::to_string(i));
//     }
//
//     db->printCacheHit();
//
//     // Clean up
//     db->Close();
//     fs::remove_all("test_db");
// }


// TEST(VeloxDBBenchMarkTest, LargeScaleInsertAndSearch) {
//     int memtableSize = 10000;  // Set memtable size
//     auto db = std::make_unique<VeloxDB>(memtableSize, 3);  // BTree degree of 3
//     db->Open("test_db");
//
//     const int numEntries = 1e8;  // Insert 1 billion key-value pairs
//     std::cout << "Starting insert of " << numEntries << " key-value pairs..." << std::endl;
//
//     // Measure insert performance
//     auto startInsert = std::chrono::high_resolution_clock::now();
//
//     // Insert 1 billion key-value pairs
//     for (int i = 0; i < numEntries; ++i) {
//         db->Put(i, "value_" + std::to_string(i));
//     }
//
//     auto endInsert = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> insertDuration = endInsert - startInsert;
//
//     std::cout << "Insert completed. Time taken: " << insertDuration.count() << " seconds" << std::endl;
//     std::cout << "Average time per insert: " << (insertDuration.count() / numEntries) * 1e6 << " microseconds" << std::endl;
//
//     // Test Search performance over 1 million random searches
//     const int numSearches = 1e6;  // 1 million search operations
//     std::cout << "Starting " << numSearches << " search operations over a dataset of " << numEntries << " entries..." << std::endl;
//
//     auto startSearch = std::chrono::high_resolution_clock::now();
//
//     // Perform 1 million search operations
//     for (int i = 0; i < numSearches; ++i) {
//         int randomKey = rand() % numEntries;  // Generate a random key in the range [0, 1e9)
//         KeyValueWrapper result = db->Get(randomKey);
//
//         // Optionally verify correctness
//         ASSERT_EQ(result.kv.string_value(), "value_" + std::to_string(randomKey));  // Verify correctness of the result
//     }
//
//     auto endSearch = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> searchDuration = endSearch - startSearch;
//
//     std::cout << "Search completed. Time taken: " << searchDuration.count() << " seconds" << std::endl;
//     std::cout << "Average time per search: " << (searchDuration.count() / numSearches) * 1e6 << " microseconds" << std::endl;
//
//     // Print cache hit information
//     db->printCacheHit();
//
//     // Clean up
//     db->Close();
//     fs::remove_all("test_db");
// }
