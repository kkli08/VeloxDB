//
// Created by damian on 9/24/24.
//

#ifndef PAGEMANAGER_H
#define PAGEMANAGER_H

#include "Page.h"
#include "BufferPool.h"
#include <string>
#include <fstream>
#include <cstdint>
#include <unordered_map>

class PageManager {
public:
    // Constructor
    PageManager(const std::string& fileName, size_t pageSize = 4096);

    // Destructor
    ~PageManager();

    // Allocate a new page, returns the page offset
    uint64_t allocatePage();

    // Write a page to disk at the given offset
    void writePage(uint64_t offset, const Page& page);

    // Read a page from disk at the given offset
    Page readPage(uint64_t offset);

    // Get the current end of file offset
    uint64_t getEOFOffset() const;

    // Close the file
    void close();

    // BufferPool configuration
    void setBufferPoolParameters(size_t capacity, EvictionPolicy policy);
    long long getCacheHit() const {return bufferPool->getCacheHit();};

private:
    std::string fileName;
    size_t pageSize;
    std::fstream file;
    uint64_t nextPageOffset;

    std::shared_ptr<BufferPool> bufferPool;
    // Methods to manage file I/O
    void openFile();
};

#endif // PAGEMANAGER_H

