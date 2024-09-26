//
// Created by Damian Li on 2024-09-20.
//

// BufferPool.h

#ifndef BUFFERPOOL_H
#define BUFFERPOOL_H

#include <string>
#include <unordered_map>
#include <list>
#include <vector>
#include <memory>
#include <mutex>
#include <random>
#include "Page.h"

enum class EvictionPolicy {
    LRU,
    CLOCK,
    RANDOM
};

struct PageId {
    std::string sstFileName;
    uint64_t pageNumber;

    bool operator==(const PageId& other) const {
        return sstFileName == other.sstFileName && pageNumber == other.pageNumber;
    }
};

// Hash function for PageId
namespace std {
    template<>
    struct hash<PageId> {
        size_t operator()(const PageId& pid) const {
            size_t h1 = std::hash<std::string>{}(pid.sstFileName);
            size_t h2 = std::hash<uint64_t>{}(pid.pageNumber);
            return h1 ^ (h2 << 1); // Combine hashes
        }
    };
}

class BufferPool {
public:
    BufferPool(size_t capacity, EvictionPolicy policy);
    ~BufferPool();

    // Retrieve a page from the buffer pool
    std::shared_ptr<Page> getPage(const std::string& sstFileName, uint64_t pageNumber);

    // Insert or update a page in the buffer pool
    void putPage(const std::string& sstFileName, uint64_t pageNumber, const std::shared_ptr<Page>& page);

    // Set the eviction policy
    void setEvictionPolicy(EvictionPolicy policy);

private:
    size_t capacity;
    EvictionPolicy policy;

    // Underlying container for the buffer pool
    std::unordered_map<PageId, std::shared_ptr<Page>> pageTable;

    // For LRU policy
    std::list<PageId> lruList;
    std::unordered_map<PageId, std::list<PageId>::iterator> lruMap;

    // For CLOCK policy
    struct ClockEntry {
        PageId pageId;
        bool referenceBit;
    };
    std::vector<ClockEntry> clockEntries;
    size_t clockHand;

    // For RANDOM policy
    std::vector<PageId> randomPool;
    std::mt19937 rng;

    // Mutex for thread safety
    std::mutex mutex;

    // Eviction functions
    void evictIfNeeded();
    void evictLRU();
    void evictClock();
    void evictRandom();

    // Update access information based on eviction policy
    void updateAccessLRU(const PageId& pageId);
    void updateAccessClock(const PageId& pageId);
    void updateAccessRandom(const PageId& pageId);
};

#endif // BUFFERPOOL_H

