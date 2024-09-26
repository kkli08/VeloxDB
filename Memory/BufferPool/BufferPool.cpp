//
// Created by Damian Li on 2024-09-20.
//

#include "BufferPool.h"

// Constructor
BufferPool::BufferPool(size_t capacity, EvictionPolicy policy)
    : capacity(capacity), policy(policy), clockHand(0), rng(std::random_device{}()) {
    if (policy == EvictionPolicy::CLOCK) {
        clockEntries.resize(capacity);
    }
}

// Destructor
BufferPool::~BufferPool() {
    // Clean up resources if necessary
}

// Retrieve a page from the buffer pool
std::shared_ptr<Page> BufferPool::getPage(const std::string& sstFileName, uint64_t pageNumber) {
    std::lock_guard<std::mutex> lock(mutex);
    PageId pageId{sstFileName, pageNumber};

    auto it = pageTable.find(pageId);
    if (it != pageTable.end()) {
        // Update access info based on policy
        switch (policy) {
            case EvictionPolicy::LRU:
                updateAccessLRU(pageId);
                break;
            case EvictionPolicy::CLOCK:
                updateAccessClock(pageId);
                break;
            case EvictionPolicy::RANDOM:
                updateAccessRandom(pageId);
                break;
        }
        return it->second;
    } else {
        return nullptr;
    }
}

// Insert or update a page in the buffer pool
void BufferPool::putPage(const std::string& sstFileName, uint64_t pageNumber, const std::shared_ptr<Page>& page) {
    std::lock_guard<std::mutex> lock(mutex);
    PageId pageId{sstFileName, pageNumber};

    if (pageTable.size() >= capacity) {
        evictIfNeeded();
    }

    pageTable[pageId] = page;

    // Update access info based on policy
    switch (policy) {
        case EvictionPolicy::LRU:
            lruList.push_front(pageId);
            lruMap[pageId] = lruList.begin();
            break;
        case EvictionPolicy::CLOCK:
            // Insert into clock entries
            clockEntries[clockHand] = {pageId, true};
            clockHand = (clockHand + 1) % capacity;
            break;
        case EvictionPolicy::RANDOM:
            randomPool.push_back(pageId);
            break;
    }
}

// Evict pages if needed
void BufferPool::evictIfNeeded() {
    switch (policy) {
        case EvictionPolicy::LRU:
            evictLRU();
            break;
        case EvictionPolicy::CLOCK:
            evictClock();
            break;
        case EvictionPolicy::RANDOM:
            evictRandom();
            break;
    }
}

// LRU Eviction
void BufferPool::evictLRU() {
    if (!lruList.empty()) {
        PageId evictPageId = lruList.back();
        lruList.pop_back();
        lruMap.erase(evictPageId);
        pageTable.erase(evictPageId);
    }
}

// Update access for LRU
void BufferPool::updateAccessLRU(const PageId& pageId) {
    auto it = lruMap.find(pageId);
    if (it != lruMap.end()) {
        lruList.erase(it->second);
        lruList.push_front(pageId);
        lruMap[pageId] = lruList.begin();
    }
}

// CLOCK Eviction
void BufferPool::evictClock() {
    while (true) {
        ClockEntry& entry = clockEntries[clockHand];
        if (!entry.referenceBit) {
            // Evict this page
            pageTable.erase(entry.pageId);
            entry.pageId = PageId{"", 0}; // Reset
            entry.referenceBit = false;
            // Move hand to next position
            clockHand = (clockHand + 1) % capacity;
            break;
        } else {
            // Clear reference bit and move hand
            entry.referenceBit = false;
            clockHand = (clockHand + 1) % capacity;
        }
    }
}

// Update access for CLOCK
void BufferPool::updateAccessClock(const PageId& pageId) {
    for (auto& entry : clockEntries) {
        if (entry.pageId == pageId) {
            entry.referenceBit = true;
            break;
        }
    }
}

// RANDOM Eviction
void BufferPool::evictRandom() {
    if (!randomPool.empty()) {
        std::uniform_int_distribution<size_t> dist(0, randomPool.size() - 1);
        size_t idx = dist(rng);
        PageId evictPageId = randomPool[idx];
        randomPool.erase(randomPool.begin() + idx);
        pageTable.erase(evictPageId);
    }
}

// Update access for RANDOM (no action needed)
void BufferPool::updateAccessRandom(const PageId& pageId) {
    // No need to update access for RANDOM policy
}

// Set the eviction policy
void BufferPool::setEvictionPolicy(EvictionPolicy newPolicy) {
    std::lock_guard<std::mutex> lock(mutex);
    policy = newPolicy;
    // Reinitialize policy-specific data structures if necessary
}

