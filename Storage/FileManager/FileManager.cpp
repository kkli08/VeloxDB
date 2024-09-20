//
// Created by Damian Li on 2024-09-08.
//

#include "FileManager.h"
#include <stdexcept>
#include <functional>
#include <cstdint>

// Constructor
FileManager::FileManager() : directory("defaultDB") {}

// Constructor with custom directory
FileManager::FileManager(fs::path dir) : directory(std::move(dir)) {
    if (!fs::exists(directory)) {
        fs::create_directories(directory);  // Ensure the directory exists
    }
}

// Set directory for SST files
void FileManager::setDirectory(const fs::path& path) {
    directory = path;
}

// Get directory for SST files
fs::path FileManager::getDirectory() const {
    return directory;
}

// Generate a unique filename for an SST file
std::string FileManager::generateSstFilename() {
    return "sst_" + std::to_string(increaseFileCounter()) + ".sst";
}

/*
 * SSTHeader Methods
 */
uint32_t SSTHeader::calculateChecksum() const {
    return sizeof(num_key_values) + sizeof(header_checksum);
}

void SSTHeader::serialize(std::ofstream& file) const {
    file.write(reinterpret_cast<const char*>(&num_key_values), sizeof(num_key_values));
    file.write(reinterpret_cast<const char*>(&header_checksum), sizeof(header_checksum));
}

SSTHeader SSTHeader::deserialize(std::ifstream& file) {
    SSTHeader header;
    file.read(reinterpret_cast<char*>(&header.num_key_values), sizeof(header.num_key_values));
    file.read(reinterpret_cast<char*>(&header.header_checksum), sizeof(header.header_checksum));
    return header;
}

/*
 * FileManager Methods
 */
FlushSSTInfo FileManager::flushToDisk(const std::vector<KeyValueWrapper>& kv_pairs) {
    FlushSSTInfo flushInfo;
    flushInfo.fileName = generateSstFilename();

    // Ensure the directory exists
    if (!fs::exists(directory)) {
        throw std::runtime_error("FileManager::flushToDisk() >>>> Directory does not exist: " + directory.string());
    }

    // Open file for binary writing
    std::ofstream file(directory / flushInfo.fileName, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("FileManager::flushToDisk() >>>> Could not open SST file for writing.");
    }

    if (kv_pairs.empty()) {
        return flushInfo;  // If the key-value pairs are empty, return early
    }

    // Set smallest and largest keys
    flushInfo.smallest_key = kv_pairs.front();
    flushInfo.largest_key = kv_pairs.back();

    // Create the header
    sstHeader.num_key_values = kv_pairs.size();
    sstHeader.header_checksum = sstHeader.calculateChecksum();

    // Write the header to the file
    sstHeader.serialize(file);

    // Serialize each KeyValueWrapper to the file with a size delimiter
    for (const auto& kv : kv_pairs) {
        // Serialize the KeyValueWrapper to a string
        std::string serialized_message;
        kv.kv.SerializeToString(&serialized_message);

        // Write the size of the serialized message first
        uint32_t size = serialized_message.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));

        // Write the serialized message
        file.write(serialized_message.data(), size);
    }

    file.close();  // Close the file after writing
    return flushInfo;
}




RedBlackTree* FileManager::loadFromDisk(const std::string& sst_filename) {
    // Open the file for binary reading
    std::ifstream file(directory / sst_filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("FileManager::loadFromDisk() >>>> Could not open SST file for reading.");
    }

    // Check file size to handle empty files
    file.seekg(0, std::ios::end);
    std::streampos file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (file_size == 0) {
        file.close();
        return new RedBlackTree();  // Return an empty RedBlackTree if the file is empty
    }

    // Deserialize the SST header
    SSTHeader sstHeader = SSTHeader::deserialize(file);
    // std::cout << "Header num_key_values: " << sstHeader.num_key_values << std::endl;

    // Create a new RedBlackTree to store the deserialized key-value pairs
    auto* tree = new RedBlackTree();

    // Deserialize each KeyValueWrapper using length-delimited reading
    for (uint32_t i = 0; i < sstHeader.num_key_values; ++i) {
        // Check the stream state
        if (!file.good()) {
            std::cerr << "FileManager::loadFromDisk() >>>> File stream is not valid during deserialization." << std::endl;
            break;
        }

        // Read the size of the next serialized message
        uint32_t size;
        file.read(reinterpret_cast<char*>(&size), sizeof(size));

        // Read the serialized message based on the size
        std::string serialized_message(size, '\0');
        file.read(&serialized_message[0], size);

        // Parse the serialized message into a KeyValueWrapper
        KeyValueWrapper kvWrapper;
        kvWrapper.kv.ParseFromString(serialized_message);

        // Insert the deserialized KeyValueWrapper into the RedBlackTree
        tree->insert(kvWrapper);

        // std::cout << "Deserialized Key: " << kvWrapper.kv.int_key() << ", Value: " << kvWrapper.kv.int_value() << std::endl;
    }

    file.close();  // Close the file after reading
    return tree;
}






