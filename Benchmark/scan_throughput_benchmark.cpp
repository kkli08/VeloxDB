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
constexpr size_t MB = 1024 * 1024 / 128; // 1MB in bytes
constexpr size_t START_DATA_SIZE_MB = 1;  // Start with 1 MB
constexpr size_t END_DATA_SIZE_MB = 2048;  // End with 512 MB (adjust as needed)
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

// Function to benchmark Scan operation
void benchmarkScan(size_t dataSizeMB, size_t memtableSize, std::ofstream& csvFile) {
    std::cout << "Benchmarking Scan: MemtableSize = " << memtableSize / MB
              << "MB, DataSize = " << dataSizeMB << "MB" << std::endl;

    // Create the database object with the specified memtable size
    auto db = std::make_unique<VeloxDB>(memtableSize, 3);  // Adjust other parameters as needed

    // Open the database
    db->Open(DB_NAME);

    // Insert data to populate the database
    size_t bytesInserted = 0;
    int counter = 1;  // Start key value
    while (bytesInserted < dataSizeMB * MB) {
        std::string value = generateRandomString(128);  // 100-byte value
        db->Put(counter, value);
        bytesInserted += sizeof(counter) + value.size();  // Calculate size
        counter++;
    }

    // Start timing the Scan operation
    auto start = high_resolution_clock::now();

    // Perform a scan operation on a range (1 to counter/2)
    auto resultSet = db->Scan(KeyValueWrapper(1, ""), KeyValueWrapper(counter / 2, ""));

    // Stop timing
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start).count();

    // Calculate throughput in MB/s
    double totalSize = static_cast<double>(resultSet.size() * (sizeof(int) + 100));  // Each entry is key (int) + value (100 bytes)
    double throughput = (totalSize / MB) / (duration / 1000.0);  // MB/s

    // Write result to CSV
    csvFile << memtableSize / MB << "," << dataSizeMB << "," << throughput << std::endl;

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
    std::string outputDir = "./scan_throughput";
    std::string outputFilePath = outputDir + "/scan_throughput.csv";

    // Create the directory if it does not exist
    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    }

    // Open CSV file for writing
    std::ofstream csvFile(outputFilePath);
    csvFile << "MemtableSizeMB,DataSizeMB,Throughput(MB/s)\n";

    // Benchmark configurations
    std::vector<size_t> memtableSizes = {25 * MB, 50 * MB, 100 * MB}; // Memtable sizes: 25MB, 50MB, 100MB

    // Run benchmarks for each Memtable size and data size
    for (auto memtableSize : memtableSizes) {
        for (size_t dataSizeMB = START_DATA_SIZE_MB; dataSizeMB <= END_DATA_SIZE_MB; dataSizeMB *= 2) {
            benchmarkScan(dataSizeMB, memtableSize, csvFile);
        }
    }

    csvFile.close();
    std::cout << "Benchmark completed. Results saved to " << outputFilePath << std::endl;
    return 0;
}
