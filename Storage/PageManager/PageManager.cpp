//
// Created by damian on 9/24/24.
//

#include "PageManager.h"
#include <iostream>
#include <stdexcept>

// Constructor
PageManager::PageManager(const std::string& fileName, size_t pageSize)
    : fileName(fileName), pageSize(pageSize), bufferPool(std::make_shared<BufferPool>(1000, EvictionPolicy::LRU)) {
    openFile();
    // Move to the end to find the next available offset
    file.seekg(0, std::ios::end);
    nextPageOffset = file.tellg();
    if (nextPageOffset % pageSize != 0) {
        nextPageOffset += pageSize - (nextPageOffset % pageSize);
    }
    // If the file is empty, set nextPageOffset to pageSize to skip metadata page at offset 0
    if (nextPageOffset == 0) {
        nextPageOffset = pageSize;
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

    // Update buffer pool
    auto pagePtr = std::make_shared<Page>(page);
    bufferPool->putPage(fileName, offset, pagePtr);
}

// Read a page from disk at the given offset
Page PageManager::readPage(uint64_t offset) {
    // Generate PageId
    PageId pageId{fileName, offset};

    // Try to get the page from buffer pool
    auto page = bufferPool->getPage(fileName, offset);
    if (page != nullptr) {
        return *page;
    } else {
        // Read from disk
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

void PageManager::setBufferPoolParameters(size_t capacity, EvictionPolicy policy) {
    bufferPool = std::make_shared<BufferPool>(capacity, policy);
}



