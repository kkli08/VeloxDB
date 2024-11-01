//
// Created by Damian Li on 2024-10-04.
//
#include <iostream>
#include <chrono>
#include <memory>
#include <string>
#include <fstream>
#include <cstdlib>
#include <filesystem>  // Include filesystem
#include "VeloxDB.h"

namespace fs = std::filesystem;
using namespace std::chrono;

// Constants for benchmark
constexpr size_t MB = 1024 * 1024 / 128; // 1MB in bytes divided by the lens of key value pair
constexpr size_t START_DATA_SIZE_MB = 128;  // Start with 1 MB
constexpr size_t END_DATA_SIZE_MB = 4096;  // End with 512 MB (adjust as needed)
const std::string DB_NAME = "benchmark_db";

// Function to generate random strings
std::string generateRandomString(size_t length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += charset[rand() % (sizeof(charset) - 1)];
    }
    return result;
}

// Function to benchmark Get operation
void benchmarkGet(size_t dataSizeMB, size_t memtableSize, std::ofstream& csvFile) {
    std::cout << "Benchmarking Get: MemtableSize = " << memtableSize / MB
              << "MB, DataSize = " << dataSizeMB << "MB" << std::endl;

    // Create the database object with the specified memtable size
    auto db = std::make_unique<VeloxDB>(memtableSize);  // Adjust other parameters as needed

    // Open the database
    db->Open(DB_NAME);

    // Insert data first to populate the database
    size_t bytesInserted = 0;
    std::vector<std::string> keys;
    while (bytesInserted < dataSizeMB * MB) {
        std::string key = generateRandomString(28);    // 28-byte key
        std::string value = generateRandomString(100); // 100-byte value
        db->Put(key, value);
        keys.push_back(key);  // Save the key for later retrieval
        bytesInserted += key.size() + value.size();
    }

    // Start timing the Get operations
    auto start = high_resolution_clock::now();

    // Retrieve all inserted keys
    for (const auto& key : keys) {
        auto result = db->Get(key);
    }

    // Stop timing
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start).count();

    // Calculate average latency in milliseconds
    double average_latency = static_cast<double>(duration) / keys.size();

    // Write result to CSV
    csvFile << memtableSize / MB << "," << dataSizeMB << "," << average_latency << std::endl;

    // Close the database
    db->Close();

    // Delete the database files to free up space
    try {
        if (fs::exists(DB_NAME)) {
            fs::remove_all(DB_NAME);
            std::cout << "Deleted database directory: " << DB_NAME << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error deleting database directory: " << e.what() << std::endl;
    }
}

int main() {
    // Seed the random number generator
    srand(static_cast<unsigned>(time(nullptr)));

    // Define the output directory for the CSV file
    std::string outputDir = "./get_latency";
    std::string outputFilePath = outputDir + "/get_latency.csv";

    // Create the directory if it does not exist
    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    }

    // Open CSV file for writing
    std::ofstream csvFile(outputFilePath);
    csvFile << "MemtableSizeMB,DataSizeMB,AverageLatency(ms)\n";

    // Benchmark configurations
    std::vector<size_t> memtableSizes = {25 * MB, 50 * MB, 100 * MB}; // Memtable sizes: 25MB, 50MB, 100MB

    // Run benchmarks for each Memtable size and data size
    for (auto memtableSize : memtableSizes) {
        for (size_t dataSizeMB = START_DATA_SIZE_MB; dataSizeMB <= END_DATA_SIZE_MB; dataSizeMB *= 2) {
            benchmarkGet(dataSizeMB, memtableSize, csvFile);
        }
    }

    csvFile.close();
    std::cout << "Benchmark completed. Results saved to " << outputFilePath << std::endl;
    return 0;
}
