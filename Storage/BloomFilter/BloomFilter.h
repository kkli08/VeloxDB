//
// Created by Damian Li on 2024-10-08.
//

//
// BloomFilter.h
//

#ifndef BLOOM_FILTER_H
#define BLOOM_FILTER_H

#include "KeyValue.h"
#include <vector>
#include <cstdint>
#include <string>
#include <cmath>
#include <stdexcept>

class BloomFilter {
public:
    // Constructor taking m (number of bits) and n (expected number of elements)
    BloomFilter(size_t m, size_t n);

    // Default constructor for deserialization
    BloomFilter();

    // Add a key to the Bloom filter
    void add(const KeyValueWrapper& kv);

    // Check if a key is possibly in the Bloom filter
    bool possiblyContains(const KeyValueWrapper& kv) const;

    // Serialization and deserialization
    std::vector<char> serialize() const;
    void deserialize(const std::vector<char>& data);

    // Getters for testing and internal use
    size_t getNumBits() const { return numBits; }
    size_t getNumHashFuncs() const { return numHashFuncs; }

    // Get the estimated size of the serialized Bloom filter
    size_t getSerializedSize() const;

private:
    size_t numBits;       // m - number of bits in the filter
    size_t numHashFuncs;  // k - number of hash functions
    size_t expectedElements; // n - expected number of elements

    std::vector<uint8_t> bitArray; // Bit array representing the filter

    // Hash function that only considers the key
    std::vector<size_t> hash(const KeyValueWrapper& kv) const;
};

#endif // BLOOM_FILTER_H




