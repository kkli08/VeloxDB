//
// Created by Damian Li on 2024-10-29.
//
// BloomFilterTest.cpp
//

#include <gtest/gtest.h>
#include "BloomFilter.h"
#include "KeyValue.h"
#include <vector>
#include <string>
#include <cmath>
#include <stdexcept>

// Test the constructor and parameter initialization
TEST(BloomFilterTest, ConstructorTest) {
    size_t m = 1000; // Number of bits
    size_t n = 100;  // Expected number of elements

    BloomFilter bf(m, n);

    // Calculate expected number of hash functions
    double k = (static_cast<double>(m) / n) * std::log(2.0);
    size_t expectedK = static_cast<size_t>(std::round(k));
    if (expectedK == 0) {
        expectedK = 1;
    }

    // Access numBits and numHashFuncs via getters
    EXPECT_EQ(bf.getNumBits(), m);
    EXPECT_EQ(bf.getNumHashFuncs(), expectedK);
}

// Test adding an element and checking possiblyContains
TEST(BloomFilterTest, AddAndCheckElement) {
    BloomFilter bf(1000, 100);

    KeyValueWrapper kv1(1, 100);
    bf.add(kv1);

    EXPECT_TRUE(bf.possiblyContains(kv1));

    KeyValueWrapper kv2(2, 200);
    EXPECT_FALSE(bf.possiblyContains(kv2));
}

// Test adding multiple elements and checking possiblyContains
TEST(BloomFilterTest, AddMultipleElements) {
    BloomFilter bf(1000, 100);

    std::vector<KeyValueWrapper> elements;
    for (int i = 0; i < 50; ++i) {
        elements.emplace_back(i, i * 10);
        bf.add(elements.back());
    }

    std::vector<KeyValueWrapper> search_elements;
    for (int i = 0; i < 50; ++i) {
        search_elements.emplace_back(i, 0);
    }

    // Check that all added elements are possibly contained
    for (const auto& kv : search_elements) {
        EXPECT_TRUE(bf.possiblyContains(kv));
    }

    // Check that an element not added is probably not contained
    KeyValueWrapper kvNotAdded(1000, 10000);
    EXPECT_FALSE(bf.possiblyContains(kvNotAdded));
}

// Test false positives
TEST(BloomFilterTest, FalsePositives) {
    size_t m = 1000;
    size_t n = 50;

    BloomFilter bf(m, n);

    // Add n elements
    for (int i = 0; i < n; ++i) {
        bf.add(KeyValueWrapper(i, i * 10));
    }

    // Check false positive rate
    int falsePositives = 0;
    int tests = 1000;
    for (int i = n + 1; i <= n + tests; ++i) {
        KeyValueWrapper kv(i, i * 10);
        if (bf.possiblyContains(kv)) {
            ++falsePositives;
        }
    }

    double fpRate = static_cast<double>(falsePositives) / tests;
    double expectedFpRate = std::pow(1 - std::exp(-static_cast<double>(bf.getNumHashFuncs()) * n / m), bf.getNumHashFuncs());

    // Allow some tolerance
    EXPECT_NEAR(fpRate, expectedFpRate, 0.05); // 5% tolerance
}

// Test serialization and deserialization
TEST(BloomFilterTest, SerializeDeserialize) {
    BloomFilter bf(1000, 100);

    for (int i = 0; i < 50; ++i) {
        bf.add(KeyValueWrapper(i, i * 10));
    }

    std::vector<char> data = bf.serialize();

    BloomFilter bf2;
    bf2.deserialize(data);

    // Check that all added elements are possibly contained in the deserialized filter
    for (int i = 0; i < 50; ++i) {
        KeyValueWrapper kv(i, i * 10);
        EXPECT_TRUE(bf2.possiblyContains(kv));
    }

    // Check that an element not added is probably not contained
    KeyValueWrapper kvNotAdded(1000, 10000);
    EXPECT_FALSE(bf2.possiblyContains(kvNotAdded));
}

// Test adding elements beyond expected n
TEST(BloomFilterTest, OverCapacity) {
    size_t m = 1000;
    size_t n = 1000;

    BloomFilter bf(m, n);

    // Add 2n elements
    for (int i = 0; i < 2 * n; ++i) {
        bf.add(KeyValueWrapper(i, i * 10));
    }

    // Check that all added elements are possibly contained
    for (int i = 0; i < 2 * n; ++i) {
        KeyValueWrapper kv(i, i * 10);
        EXPECT_TRUE(bf.possiblyContains(kv));
    }

    // False positive rate may be higher due to overcapacity
    int falsePositives = 0;
    int tests = 1000;
    for (int i = 2 * n + 1; i <= 2 * n + tests; ++i) {
        KeyValueWrapper kv(i, i * 10);
        if (bf.possiblyContains(kv)) {
            ++falsePositives;
        }
    }

    double fpRate = static_cast<double>(falsePositives) / tests;

    // Expected false positive rate with overcapacity
    double expectedFpRate = std::pow(1 - std::exp(-static_cast<double>(bf.getNumHashFuncs()) * (2 * n) / m), bf.getNumHashFuncs());

    // Allow some tolerance
    EXPECT_NEAR(fpRate, expectedFpRate, 0.05);
}

// Test constructing BloomFilter with invalid parameters
TEST(BloomFilterTest, InvalidParameters) {
    // m = 0
    EXPECT_THROW(BloomFilter bf(0, 100), std::invalid_argument);

    // n = 0
    EXPECT_THROW(BloomFilter bf(1000, 0), std::invalid_argument);

    // m and n both 0
    EXPECT_THROW(BloomFilter bf(0, 0), std::invalid_argument);
}

// Test that the number of hash functions k is calculated correctly
TEST(BloomFilterTest, NumberOfHashFunctions) {
    size_t m = 1000;
    size_t n = 100;

    BloomFilter bf(m, n);

    double k = (static_cast<double>(m) / n) * std::log(2.0);
    size_t expectedK = static_cast<size_t>(std::round(k));
    if (expectedK == 0) {
        expectedK = 1;
    }

    EXPECT_EQ(bf.getNumHashFuncs(), expectedK);
}

// Test that the BloomFilter can handle large m and n
TEST(BloomFilterTest, LargeFilter) {
    size_t m = 1000000;
    size_t n = 100000;

    BloomFilter bf(m, n);

    // Add n elements
    for (int i = 0; i < n; ++i) {
        bf.add(KeyValueWrapper(i, i * 10));
    }

    // Check that all added elements are possibly contained
    for (int i = 0; i < n; i += n / 100) { // Check every 1% of elements
        KeyValueWrapper kv(i, i * 10);
        EXPECT_TRUE(bf.possiblyContains(kv));
    }

    // Check false positive rate
    int falsePositives = 0;
    int tests = 1000;
    for (int i = n + 1; i <= n + tests; ++i) {
        KeyValueWrapper kv(i, i * 10);
        if (bf.possiblyContains(kv)) {
            ++falsePositives;
        }
    }

    double fpRate = static_cast<double>(falsePositives) / tests;
    double expectedFpRate = std::pow(1 - std::exp(-static_cast<double>(bf.getNumHashFuncs()) * n / m), bf.getNumHashFuncs());

    // Allow some tolerance
    EXPECT_NEAR(fpRate, expectedFpRate, 0.05); // 5% tolerance
}

// Test that adding the same element multiple times doesn't affect the Bloom filter
TEST(BloomFilterTest, AddDuplicateElements) {
    BloomFilter bf(1000, 100);

    KeyValueWrapper kv(42, 420);

    bf.add(kv);
    bf.add(kv);
    bf.add(kv);

    EXPECT_TRUE(bf.possiblyContains(kv));

    // False positives should not increase due to duplicates
    int falsePositives = 0;
    int tests = 1000;
    for (int i = 1000; i < 1000 + tests; ++i) {
        KeyValueWrapper kvNotAdded(i, i * 10);
        if (bf.possiblyContains(kvNotAdded)) {
            ++falsePositives;
        }
    }

    double fpRate = static_cast<double>(falsePositives) / tests;
    double expectedFpRate = std::pow(1 - std::exp(-static_cast<double>(bf.getNumHashFuncs()) * 1 / bf.getNumBits()), bf.getNumHashFuncs());

    // Since we only added one unique element, the expected false positive rate is low
    EXPECT_NEAR(fpRate, expectedFpRate, 0.05);
}

// Test that the BloomFilter can handle different types of keys
TEST(BloomFilterTest, DifferentKeyTypes) {
    BloomFilter bf(1000, 100);

    KeyValueWrapper kvInt(1, 100);
    KeyValueWrapper kvDouble(3.14, 1.618);
    KeyValueWrapper kvString("key1", "value1");
    KeyValueWrapper kvChar('A', 'Z');

    bf.add(kvInt);
    bf.add(kvDouble);
    bf.add(kvString);
    bf.add(kvChar);

    EXPECT_TRUE(bf.possiblyContains(kvInt));
    EXPECT_TRUE(bf.possiblyContains(kvDouble));
    EXPECT_TRUE(bf.possiblyContains(kvString));
    EXPECT_TRUE(bf.possiblyContains(kvChar));

    // Check that an element not added is probably not contained
    KeyValueWrapper kvNotAdded("not_added", "no_value");
    EXPECT_FALSE(bf.possiblyContains(kvNotAdded));
}
