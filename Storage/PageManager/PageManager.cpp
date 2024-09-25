//
// Created by damian on 9/24/24.
//

#include "PageManager.h"
#include <iostream>
#include <stdexcept>

// Constructor
PageManager::PageManager(const std::string& fileName, size_t pageSize)
    : fileName(fileName), pageSize(pageSize), nextPageOffset(0) {
    openFile();
    // Move to the end to find the next available offset
    file.seekg(0, std::ios::end);
    nextPageOffset = file.tellg();
    if (nextPageOffset % pageSize != 0) {
        nextPageOffset += pageSize - (nextPageOffset % pageSize);
    }
}

// Destructor
PageManager::~PageManager() {
    if (file.is_open()) {
        file.close();
    }
}

// Open the file
void PageManager::openFile() {
    // Open the file in read/write mode, binary
    file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        // If the file doesn't exist, create it
        file.clear();
        file.open(fileName, std::ios::out | std::ios::binary);
        file.close();
        // Re-open in read/write mode
        file.open(fileName, std::ios::in | std::ios::out | std::ios::binary);
    }
    if (!file.is_open()) {
        throw std::runtime_error("PageManager: Failed to open file " + fileName);
    }
}

// Allocate a new page
uint64_t PageManager::allocatePage() {
    uint64_t offset = nextPageOffset;
    nextPageOffset += pageSize;
    return offset;
}

// Write a page to disk at the given offset
void PageManager::writePage(uint64_t offset, const Page& page) {
    std::vector<char> buffer = page.serialize();
    if (buffer.size() != pageSize) {
        throw std::runtime_error("PageManager: Serialized page size does not match page size");
    }
    file.seekp(offset, std::ios::beg);
    file.write(buffer.data(), pageSize);
    file.flush();
}

// Read a page from disk at the given offset
Page PageManager::readPage(uint64_t offset) {
    file.seekg(offset, std::ios::beg);
    std::vector<char> buffer(pageSize);
    file.read(buffer.data(), pageSize);
    if (!file) {
        throw std::runtime_error("PageManager: Failed to read page at offset " + std::to_string(offset));
    }
    Page page(Page::PageType::LEAF_NODE); // Placeholder, actual type will be set during deserialization
    page.deserialize(buffer);
    return page;
}

// Get the current end of file offset
uint64_t PageManager::getEOFOffset() const {
    return nextPageOffset;
}

// Close the file
void PageManager::close() {
    if (file.is_open()) {
        file.close();
    }
}
