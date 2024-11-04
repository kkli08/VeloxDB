//
// Created by Damian Li on 2024-10-08.
//

#include "BloomFilter.h"
#include <functional>
#include <cstring>
#include <stdexcept>

BloomFilter::BloomFilter(size_t m, size_t n)
    : numBits(m), expectedElements(n) {
    // Validate parameters
    if (m == 0) {
        throw std::invalid_argument("Number of bits (m) must be greater than 0");
    }
    if (n == 0) {
        throw std::invalid_argument("Expected number of elements (n) must be greater than 0");
    }

    bitArray.resize((m + 7) / 8, 0);

    // Calculate the optimal number of hash functions: k = (m / n) * ln 2
    double k = (static_cast<double>(m) / n) * std::log(2.0);
    numHashFuncs = static_cast<size_t>(std::round(k));
    if (numHashFuncs == 0) {
        numHashFuncs = 1;
    }
}

BloomFilter::BloomFilter() : numBits(0), numHashFuncs(0), expectedElements(0) {
    // Default constructor for deserialization
}

void BloomFilter::add(const KeyValueWrapper& kv) {
    auto hashIndices = hash(kv);
    for (size_t index : hashIndices) {
        index %= numBits;
        bitArray[index / 8] |= (1 << (index % 8));
    }
}

bool BloomFilter::possiblyContains(const KeyValueWrapper& kv) const {
    auto hashIndices = hash(kv);
    int i = 2;
    for (size_t index : hashIndices) {
        index %= numBits;
        if (!(bitArray[index / 8] & (1 << (index % 8)))) {
            // cout << "return false" << endl;
            return false;
        }
    }

    return true;
}

std::vector<char> BloomFilter::serialize() const {
    std::vector<char> data;

    // Serialize numBits, numHashFuncs, expectedElements
    data.resize(sizeof(numBits) + sizeof(numHashFuncs) + sizeof(expectedElements));
    size_t offset = 0;
    std::memcpy(data.data() + offset, &numBits, sizeof(numBits));
    offset += sizeof(numBits);
    std::memcpy(data.data() + offset, &numHashFuncs, sizeof(numHashFuncs));
    offset += sizeof(numHashFuncs);
    std::memcpy(data.data() + offset, &expectedElements, sizeof(expectedElements));
    offset += sizeof(expectedElements);

    // Append bitArray
    data.insert(data.end(), bitArray.begin(), bitArray.end());

    return data;
}

void BloomFilter::deserialize(const std::vector<char>& data) {
    if (data.size() < sizeof(numBits) + sizeof(numHashFuncs) + sizeof(expectedElements)) {
        throw std::runtime_error("Invalid Bloom filter data");
    }

    size_t offset = 0;
    std::memcpy(&numBits, data.data() + offset, sizeof(numBits));
    offset += sizeof(numBits);
    std::memcpy(&numHashFuncs, data.data() + offset, sizeof(numHashFuncs));
    offset += sizeof(numHashFuncs);
    std::memcpy(&expectedElements, data.data() + offset, sizeof(expectedElements));
    offset += sizeof(expectedElements);

    // Extract bitArray
    bitArray.assign(data.begin() + offset, data.end());
}

std::vector<size_t> BloomFilter::hash(const KeyValueWrapper& kv) const {
    // Use a combination of hash functions
    std::vector<size_t> hashValues(numHashFuncs);

    // Serialize the key to a string (only the key, not the value)
    std::string keyString;

    // Extract the key based on its type using key_case()
    switch (kv.kv.key_case()) {
        case KeyValue::kIntKey:
            keyString = std::to_string(kv.kv.int_key());
        break;
        case KeyValue::kLongKey:
            keyString = std::to_string(kv.kv.long_key());
        break;
        case KeyValue::kDoubleKey:
            keyString = std::to_string(kv.kv.double_key());
        break;
        case KeyValue::kStringKey:
            keyString = kv.kv.string_key();
        break;
        case KeyValue::kCharKey:
            keyString = kv.kv.char_key();
        break;
        default:
            // Handle error or empty key
                keyString = "";
        break;
    }

    // Seed for hash functions
    std::hash<std::string> hasher;
    size_t baseHash = hasher(keyString);

    // Second hash function seed
    size_t hash2Seed = std::hash<size_t>{}(baseHash);

    if (hash2Seed == 0) {
        hash2Seed = 0x27d4eb2d; // Use a non-zero seed if zero
    }

    for (size_t i = 0; i < numHashFuncs; ++i) {
        // Double Hashing: hash_i(x) = (hash1(x) + i * hash2(x)) % m
        size_t hashValue = (baseHash + i * hash2Seed) % numBits;
        hashValues[i] = hashValue;
    }

    return hashValues;
}


size_t BloomFilter::getSerializedSize() const {
    size_t size = 0;
    size += sizeof(numBits);
    size += sizeof(numHashFuncs);
    size += sizeof(expectedElements);
    size += bitArray.size();
    return size;
}

