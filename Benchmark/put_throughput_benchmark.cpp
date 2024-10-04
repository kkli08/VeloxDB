#include <iostream>
#include <chrono>
#include <memory>
#include <string>
#include <fstream>
#include <cstdlib>
#include <filesystem>  // Include filesystem
#include "../VeloxDB/VeloxDB.h"

namespace fs = std::filesystem;
using namespace std::chrono;

// Constants for benchmark
constexpr size_t MB = 1024 * 1024; // 1MB in bytes
constexpr size_t START_DATA_SIZE_MB = 1;  // Start with 1 MB
constexpr size_t END_DATA_SIZE_MB = 512;  // End with 512 MB (adjust as needed)
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

// Function to benchmark Put operation
void benchmarkPut(size_t dataSizeMB, size_t memtableSize, std::ofstream& csvFile) {
    std::cout << "Benchmarking Put: MemtableSize = " << memtableSize / MB
              << "MB, DataSize = " << dataSizeMB << "MB" << std::endl;

    // Create the database object with the specified memtable size
    auto db = std::make_unique<VeloxDB>(memtableSize, 3);  // Adjust other parameters as needed

    // Open the database
    db->Open(DB_NAME);

    // Start timing
    auto start = high_resolution_clock::now();

    // Insert data
    size_t bytesInserted = 0;
    while (bytesInserted < dataSizeMB * MB) {
        std::string key = generateRandomString(16);    // 16-byte key
        std::string value = generateRandomString(100); // 100-byte value
        db->Put(key, value);
        bytesInserted += key.size() + value.size();
    }

    // Stop timing
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start).count();

    // Calculate throughput in MB/s
    double throughput = static_cast<double>(dataSizeMB * 1000) / duration;

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
    std::string outputDir = "./put_throughput";
    std::string outputFilePath = outputDir + "/put_throughput.csv";

    // Create the directory if it does not exist
    if (!fs::exists(outputDir)) {
        fs::create_directories(outputDir);
    }

    // Open CSV file for writing
    std::ofstream csvFile(outputFilePath);
    csvFile << "MemtableSizeMB,DataSizeMB,Throughput(MB/s)\n";

    // Benchmark configurations
    std::vector<size_t> memtableSizes = {10 * MB, 15 * MB}; // Memtable sizes: 1MB, 5MB, 10MB

    // Run benchmarks for each Memtable size and data size
    for (auto memtableSize : memtableSizes) {
        for (size_t dataSizeMB = START_DATA_SIZE_MB; dataSizeMB <= END_DATA_SIZE_MB; dataSizeMB *= 2) {
            benchmarkPut(dataSizeMB, memtableSize, csvFile);
        }
    }

    csvFile.close();
    std::cout << "Benchmark completed. Results saved to " << outputFilePath << std::endl;
    return 0;
}
