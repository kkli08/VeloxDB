//
// Created by Damian Li on 2024-09-07.
//

#include <gtest/gtest.h>
#include <sstream>
#include <filesystem>
#include <fstream>
#include "KeyValue.h"

// Test for Key and Value Type Deduction
TEST(KeyValueWrapperTest, TypeDeduction) {
    KeyValueWrapper kv1(42, "Hello");
    KeyValueWrapper kv2(3.14, 'A');

    EXPECT_EQ(kv1.getKeyType(), KeyValue::INT);
    EXPECT_EQ(kv1.getValueType(), KeyValue::STRING);
    EXPECT_EQ(kv2.getKeyType(), KeyValue::DOUBLE);
    EXPECT_EQ(kv2.getValueType(), KeyValue::CHAR);
}


// Test for Key and Value Accessors
TEST(KeyValueWrapperTest, Accessors) {
    KeyValueWrapper kv(100, "TestValue");

    // Use the generated Protobuf accessors to verify the key and value
    EXPECT_EQ(kv.kv.int_key(), 100);
    EXPECT_EQ(kv.kv.string_value(), "TestValue");
}


// Test for Numeric Comparison of Keys
TEST(KeyValueWrapperTest, NumericComparison) {
    KeyValueWrapper kv1(10, "A");  // Key = 10
    KeyValueWrapper kv2(20, "B");  // Key = 20

    // Test numeric comparison (integers)
    EXPECT_TRUE(kv1 < kv2);   // 10 < 20
    EXPECT_FALSE(kv2 < kv1);  // 20 is not less than 10
}



// Test for Lexicographical Comparison of Keys
TEST(KeyValueWrapperTest, LexicographicalComparison) {
    KeyValueWrapper kv1("Apple", 1);  // Key = "Apple"
    KeyValueWrapper kv2("Banana", 2); // Key = "Banana"

    // Test lexicographical comparison (strings)
    EXPECT_TRUE(kv1 < kv2);   // "Apple" < "Banana"
    EXPECT_FALSE(kv2 < kv1);  // "Banana" is not less than "Apple"
}


// Test for Mixed Type Comparison Failure
TEST(KeyValueWrapperTest, MixedTypeComparison) {
    KeyValueWrapper kv1(100, "Test");    // Numeric key
    KeyValueWrapper kv2("Key", 50);      // String key

    EXPECT_TRUE(kv1 < kv2);  // Numeric is always smaller than string
}

// Test for Key and Value Printing
TEST(KeyValueWrapperTest, PrintKeyValue) {
    KeyValueWrapper kv(42, "Hello");

    testing::internal::CaptureStdout();
    kv.printKeyValue();
    std::string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Key: 42"), std::string::npos);
    EXPECT_NE(output.find("Value: Hello"), std::string::npos);
}

// Test for Large Integer Keys and Values
TEST(KeyValueWrapperTest, LargeIntegers) {
    long long largeKey = 9223372036854775807LL;  // Largest 64-bit signed integer
    KeyValueWrapper kv(largeKey, largeKey);

    EXPECT_EQ(kv.kv.long_key(), largeKey);
    EXPECT_EQ(kv.kv.long_value(), largeKey);
}

// Test for Double Keys and Values
TEST(KeyValueWrapperTest, DoubleValues) {
    KeyValueWrapper kv(3.14159, 2.71828);

    EXPECT_EQ(kv.kv.double_key(), 3.14159);
    EXPECT_EQ(kv.kv.double_value(), 2.71828);
}

// Test for Character Keys and Values
TEST(KeyValueWrapperTest, CharValues) {
    KeyValueWrapper kv('A', 'B');

    EXPECT_EQ(kv.kv.char_key(), "A");
    EXPECT_EQ(kv.kv.char_value(), "B");
}

TEST(KeyValueWrapperTest, CompareIntWithLong) {
    KeyValueWrapper kv1(100, 0);  // int key
    KeyValueWrapper kv2(100LL, 0);  // long long key

    EXPECT_FALSE(kv1 < kv2);
    EXPECT_FALSE(kv2 < kv1);
    EXPECT_TRUE(kv1 == kv2);
}

TEST(KeyValueWrapperTest, CompareIntWithDouble) {
    KeyValueWrapper kv1(100, 0);  // int key
    KeyValueWrapper kv2(100.0, 0);  // double key

    EXPECT_FALSE(kv1 < kv2);
    EXPECT_FALSE(kv2 < kv1);
    EXPECT_TRUE(kv1 == kv2);
}

TEST(KeyValueWrapperTest, CompareIntWithString) {
    KeyValueWrapper kv1(100, 0);  // int key
    KeyValueWrapper kv2("abc", 0);  // string key

    EXPECT_TRUE(kv1 < kv2);
    EXPECT_FALSE(kv2 < kv1);
}

TEST(KeyValueWrapperTest, CompareDoubleWithString) {
    KeyValueWrapper kv1(100.5, 0);  // double key
    KeyValueWrapper kv2("xyz", 0);  // string key

    EXPECT_TRUE(kv1 < kv2);
    EXPECT_FALSE(kv2 < kv1);
}

TEST(KeyValueWrapperTest, CompareCharWithString) {
    KeyValueWrapper kv1('a', 0);  // char key
    KeyValueWrapper kv2("abc", 0);  // string key

    EXPECT_TRUE(kv1 < kv2);
    EXPECT_FALSE(kv2 < kv1);
}

TEST(KeyValueWrapperTest, CompareCharWithDouble) {
    KeyValueWrapper kv1('a', 0);  // char key
    KeyValueWrapper kv2(99.99, 0);  // double key

    EXPECT_FALSE(kv1 < kv2);
    EXPECT_TRUE(kv2 < kv1);
}

TEST(KeyValueWrapperTest, CompareStringWithString) {
    KeyValueWrapper kv1("abc", 0);  // string key
    KeyValueWrapper kv2("xyz", 0);  // string key

    EXPECT_TRUE(kv1 < kv2);
    EXPECT_FALSE(kv2 < kv1);
}

TEST(KeyValueWrapperTest, CompareDefaultKeyValues) {
    KeyValueWrapper kv1;  // Default KeyValueWrapper (no key set)
    KeyValueWrapper kv2(0, 0);  // Explicitly initialized to 0

    // Check if kv1 is in the default (unset) state
    EXPECT_TRUE(kv1.isDefault());

    // Compare default and explicitly initialized KeyValueWrapper
    EXPECT_FALSE(kv1 == kv2);  // kv1 is default (unset), kv2 is explicitly set to 0, so they should not be equal
}


// Test the printKeyValue method
TEST(KeyValueWrapperTest, Print_KeyValue) {
    // Create a KeyValueWrapper instance
    KeyValueWrapper kv(100, "testValue");

    // Redirect the output to a stringstream
    std::ostringstream output;
    std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();  // Save old buffer
    std::cout.rdbuf(output.rdbuf());  // Redirect std::cout to stringstream

    // Call printKeyValue() which should print the key-value to the stream
    kv.printKeyValue();

    // Restore std::cout to its original buffer
    std::cout.rdbuf(oldCoutStreamBuf);

    // Expected output
    std::string expectedOutput =
        "Key Type: INT\n"
        "Key: 100\n"
        "Value Type: STRING\n"
        "Value: testValue\n";

    // Compare the redirected output to the expected output
    EXPECT_EQ(output.str(), expectedOutput);
}





namespace fs = std::filesystem;


TEST(KeyValueWrapperTest, SerializedAndDeserializeDataintoSST) {
    // Create a directory for testing, ensure it exists
    fs::create_directories("test_db");

    // Open a file for writing serialized data
    std::ofstream file("test_db/sst_1.sst", std::ios::binary);
    ASSERT_TRUE(file.is_open());

    // Create a vector of KeyValueWrapper pairs
    std::vector<KeyValueWrapper> kv_pairs = {
        KeyValueWrapper(1, 100),
        KeyValueWrapper(3.14, 1.618),
        KeyValueWrapper("key1", "value1"),
        KeyValueWrapper('A', 'Z')
    };

    // Serialize each KeyValueWrapper into the file with a size delimiter
    for (const auto& kv : kv_pairs) {
        // Write the size of the serialized message first
        std::string serialized_message;
        kv.kv.SerializeToString(&serialized_message);
        uint32_t size = serialized_message.size();
        file.write(reinterpret_cast<const char*>(&size), sizeof(size));
        file.write(serialized_message.data(), size);
    }

    file.close();  // Ensure the file is closed after writing

    // Now, open the same file for reading and deserialize the data
    std::ifstream input_file("test_db/sst_1.sst", std::ios::binary);
    ASSERT_TRUE(input_file.is_open());

    // Deserialize each KeyValueWrapper by reading size first
    std::vector<KeyValueWrapper> deserialized_kv_pairs;
    while (input_file.peek() != EOF) {
        uint32_t size;
        input_file.read(reinterpret_cast<char*>(&size), sizeof(size));  // Read the size of the next message
        std::string serialized_message(size, '\0');
        input_file.read(&serialized_message[0], size);  // Read the message data

        KeyValueWrapper kvWrapper;
        kvWrapper.kv.ParseFromString(serialized_message);  // Deserialize the KeyValueWrapper
        deserialized_kv_pairs.push_back(kvWrapper);
    }

    input_file.close();  // Ensure the file is closed after reading

    // Ensure that we have deserialized the same number of key-value pairs
    ASSERT_EQ(kv_pairs.size(), deserialized_kv_pairs.size());

    // Verify that each deserialized key-value pair matches the original
    for (size_t i = 0; i < kv_pairs.size(); ++i) {
        const auto& original = kv_pairs[i];
        const auto& deserialized = deserialized_kv_pairs[i];

        if (original.kv.key_case() == KeyValue::kIntKey) {
            EXPECT_EQ(original.kv.int_key(), deserialized.kv.int_key());
            EXPECT_EQ(original.kv.int_value(), deserialized.kv.int_value());
        } else if (original.kv.key_case() == KeyValue::kDoubleKey) {
            EXPECT_EQ(original.kv.double_key(), deserialized.kv.double_key());
            EXPECT_EQ(original.kv.double_value(), deserialized.kv.double_value());
        } else if (original.kv.key_case() == KeyValue::kStringKey) {
            EXPECT_EQ(original.kv.string_key(), deserialized.kv.string_key());
            EXPECT_EQ(original.kv.string_value(), deserialized.kv.string_value());
        } else if (original.kv.key_case() == KeyValue::kCharKey) {
            EXPECT_EQ(original.kv.char_key(), deserialized.kv.char_key());
            EXPECT_EQ(original.kv.char_value(), deserialized.kv.char_value());
        }
    }

    // Clean up: Delete the test directory and the file
    fs::remove_all("test_db");
}
