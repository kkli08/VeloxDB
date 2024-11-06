//
// Created by Damian Li on 2024-09-20.
//
// VeloxDB.cpp
#include "VeloxDB.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

// Default constructor
VeloxDB::VeloxDB()
    : memtable_size(10000), // Default memtable size
      lsmTree(std::make_unique<LSMTree>(memtable_size)) {
    // Constructor body (if needed)
}

// Constructor with memtable size
VeloxDB::VeloxDB(int memtable_size)
    : memtable_size(memtable_size),
      lsmTree(std::make_unique<LSMTree>(memtable_size)) {
    // Constructor body (if needed)
}

// Destructor
VeloxDB::~VeloxDB() {
    if (is_open) {
        Close();
    }
}

// Open the database
void VeloxDB::Open(const std::string& db_name) {
    std::cout << "Opening database " << db_name << std::endl;

    if (is_open) {
        throw std::runtime_error("Database is already open.");
    }

    // Define the path to the database directory
    fs::path db_path = db_name;
    // Check if the directory exists
    if (!fs::exists(db_path)) {
        // Directory does not exist, so create it
        if (fs::create_directory(db_path)) {
            std::cout << "Created database directory: " << db_name << std::endl;
        } else {
            std::cerr << "Failed to create directory: " << db_name << std::endl;
        }
    } else {
        std::cout << "Existing database directory: " << db_name << std::endl;
    }

    // Set the database path in LSMTree
    lsmTree->setDBPath(db_path.string());

    // Set API attribute fs::path
    set_path(db_path);

    // Set the flag to indicate the database is open
    is_open = true;
}

// Close the database
void VeloxDB::Close() {
    check_if_open();
    std::cout << "Closing database" << std::endl;

    // Save state of LSMTree
    lsmTree->saveState();

    // Set the flag to indicate the database is closed
    is_open = false;
}

// Get method
KeyValueWrapper VeloxDB::Get(const KeyValueWrapper& keyValueWrapper) {
    check_if_open();

    // Use lsmTree's get method
    return lsmTree->get(keyValueWrapper);
}

// Scan method
std::vector<KeyValueWrapper> VeloxDB::Scan(const KeyValueWrapper& small_key, const KeyValueWrapper& large_key) {
    check_if_open();

    std::vector<KeyValueWrapper> vectorResult;
    // Use lsmTree's scan method
    lsmTree->scan(small_key, large_key, vectorResult);


    return vectorResult;
}

// Helper function to set path
void VeloxDB::set_path(const fs::path& db_path) {
    path = db_path;
    // lsmTree->setDBPath(path.string()); // Already set in Open
}

// Set buffer pool parameters
void VeloxDB::setBufferPoolParameters(size_t capacity, EvictionPolicy policy) {
    bufferPoolCapacity = capacity;
    bufferPoolPolicy = policy;

    // Set buffer pool parameters in LSMTree or underlying components
    // TODO: Implement buffer pool parameters in LSMTree components
}

// Print cache hit information
void VeloxDB::printCacheHit() const {
    // Assuming LSMTree or underlying components have getTotalCacheHits method
    // TODO: Implement getTotalCacheHits in LSMTree components
    // Example:
    // std::cout << "Cache hit: " << lsmTree->getTotalCacheHits() << " times." << std::endl;
}
