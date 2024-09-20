//
// Created by Damian Li on 2024-09-07.
//
#include "KeyValue.h"
#include "KeyValue.pb.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
using namespace std;

void KeyValueWrapper::serialize(std::ostream& os) const {
    kv.SerializeToOstream(&os);  // Use Protobuf serialization
}

KeyValueWrapper KeyValueWrapper::deserialize(std::istream& is) {
    KeyValue kv;
    kv.ParseFromIstream(&is);  // Use Protobuf deserialization
    KeyValueWrapper wrapper;
    wrapper.kv = kv;
    return wrapper;
}

bool KeyValueWrapper::isEmpty() const {
    return kv.key_case() == KeyValue::KEY_NOT_SET || kv.value_case() == KeyValue::VALUE_NOT_SET;
}



// Comparison operator
bool KeyValueWrapper::operator<(const KeyValueWrapper& other) const {
    // Handle mixed numeric-type comparisons by casting both to double
    if ((kv.key_case() == KeyValue::kIntKey || kv.key_case() == KeyValue::kLongKey || kv.key_case() == KeyValue::kDoubleKey) &&
        (other.kv.key_case() == KeyValue::kIntKey || other.kv.key_case() == KeyValue::kLongKey || other.kv.key_case() == KeyValue::kDoubleKey)) {

        double thisKey = (kv.key_case() == KeyValue::kIntKey) ? kv.int_key() :
                         (kv.key_case() == KeyValue::kLongKey) ? kv.long_key() :
                         kv.double_key();
        double otherKey = (other.kv.key_case() == KeyValue::kIntKey) ? other.kv.int_key() :
                          (other.kv.key_case() == KeyValue::kLongKey) ? other.kv.long_key() :
                          other.kv.double_key();
        return thisKey < otherKey;
        }

    // Handle comparison between numeric (int, long, double) and char/string types
    switch (kv.key_case()) {
        case KeyValue::kIntKey:
        case KeyValue::kLongKey:
        case KeyValue::kDoubleKey:
            // Numeric types are always smaller than char and string
            if (other.kv.key_case() == KeyValue::kCharKey || other.kv.key_case() == KeyValue::kStringKey) {
                return true;
            }
            break;

        case KeyValue::kCharKey:
            // char is always smaller than string, but larger than numeric types
            if (other.kv.key_case() == KeyValue::kStringKey) {
                return true;
            }
            if (other.kv.key_case() == KeyValue::kIntKey || other.kv.key_case() == KeyValue::kLongKey || other.kv.key_case() == KeyValue::kDoubleKey) {
                return false;
            }
            break;

        case KeyValue::kStringKey:
            // string is always larger than numeric and char types
            if (other.kv.key_case() == KeyValue::kIntKey || other.kv.key_case() == KeyValue::kLongKey || other.kv.key_case() == KeyValue::kDoubleKey || other.kv.key_case() == KeyValue::kCharKey) {
                return false;
            }
            break;

        default:
            throw std::invalid_argument("KeyValueWrapper::operator<() : Line 73 : Unsupported key type for comparison.");
    }



    // Handle same-type comparisons
    switch (kv.key_case()) {
        case KeyValue::kIntKey:
            return kv.int_key() < other.kv.int_key();
        case KeyValue::kLongKey:
            return kv.long_key() < other.kv.long_key();
        case KeyValue::kDoubleKey:
            return kv.double_key() < other.kv.double_key();
        case KeyValue::kCharKey:
            return kv.char_key() < other.kv.char_key();
        case KeyValue::kStringKey:
            return kv.string_key() < other.kv.string_key();
        default:
            throw std::invalid_argument("KeyValueWrapper::operator<() : Line 91 : Invalid key type for comparison.");
    }
}








// Define operator> in terms of operator<
bool KeyValueWrapper::operator>(const KeyValueWrapper& other) const {
    return other < *this;
}


// Define operator<= in terms of operator<
bool KeyValueWrapper::operator<=(const KeyValueWrapper& other) const {
    return !(other < *this);
}


// Define operator>= in terms of operator<
bool KeyValueWrapper::operator>=(const KeyValueWrapper& other) const {
    return !(*this < other);
}


// Comparison operator for key equality
// Only for key
bool KeyValueWrapper::operator==(const KeyValueWrapper& other) const {
    // If one of the keys is unset (default state), they can't be equal
    if (kv.key_case() == KeyValue::KEY_NOT_SET || other.kv.key_case() == KeyValue::KEY_NOT_SET) {
        return false;
    }

    // First check if both keys have the same type or are numeric
    if (kv.key_case() == other.kv.key_case()) {
        // If both keys are of the same type, compare their values
        switch (kv.key_case()) {
            case KeyValue::kIntKey:
                return kv.int_key() == other.kv.int_key();
            case KeyValue::kLongKey:
                return kv.long_key() == other.kv.long_key();
            case KeyValue::kDoubleKey:
                return kv.double_key() == other.kv.double_key();
            case KeyValue::kStringKey:
                return kv.string_key() == other.kv.string_key();
            case KeyValue::kCharKey:
                return kv.char_key() == other.kv.char_key();
            default:
                throw std::invalid_argument("Invalid key type for comparison.");
        }
    } else if (
        (kv.key_case() == KeyValue::kIntKey || kv.key_case() == KeyValue::kLongKey || kv.key_case() == KeyValue::kDoubleKey) &&
        (other.kv.key_case() == KeyValue::kIntKey || other.kv.key_case() == KeyValue::kLongKey || other.kv.key_case() == KeyValue::kDoubleKey)
    ) {
        // If both keys are numeric but of different types, cast to double and compare
        double thisKey = (kv.key_case() == KeyValue::kIntKey) ? kv.int_key() :
                         (kv.key_case() == KeyValue::kLongKey) ? kv.long_key() :
                         kv.double_key();
        double otherKey = (other.kv.key_case() == KeyValue::kIntKey) ? other.kv.int_key() :
                          (other.kv.key_case() == KeyValue::kLongKey) ? other.kv.long_key() :
                          other.kv.double_key();
        return thisKey == otherKey;
    }

    // If the types are different and not numeric, they are not equal
    return false;
}





// Print key-value
void KeyValueWrapper::printKeyValue() const {
    std::cout << "Key Type: " << keyValueTypeToString(kv.key_type()) << std::endl;

    switch (kv.key_case()) {
        case KeyValue::kIntKey:
            std::cout << "Key: " << kv.int_key() << std::endl;
        break;
        case KeyValue::kLongKey:
            std::cout << "Key: " << kv.long_key() << std::endl;
        break;
        case KeyValue::kDoubleKey:
            std::cout << "Key: " << kv.double_key() << std::endl;
        break;
        case KeyValue::kStringKey:
            std::cout << "Key: " << kv.string_key() << std::endl;
        break;
        case KeyValue::kCharKey:
            std::cout << "Key: " << kv.char_key() << std::endl;
        break;
        default:
            std::cout << "Key: Unset" << std::endl;
    }

    std::cout << "Value Type: " << keyValueTypeToString(kv.value_type()) << std::endl;

    switch (kv.value_case()) {
        case KeyValue::kIntValue:
            std::cout << "Value: " << kv.int_value() << std::endl;
        break;
        case KeyValue::kLongValue:
            std::cout << "Value: " << kv.long_value() << std::endl;
        break;
        case KeyValue::kDoubleValue:
            std::cout << "Value: " << kv.double_value() << std::endl;
        break;
        case KeyValue::kStringValue:
            std::cout << "Value: " << kv.string_value() << std::endl;
        break;
        case KeyValue::kCharValue:
            std::cout << "Value: " << kv.char_value() << std::endl;
        break;
        default:
            std::cout << "Value: Unset" << std::endl;
    }
}



// Enum to string conversion
std::string KeyValueWrapper::keyValueTypeToString(KeyValue::KeyValueType type) const {
    switch (type) {
        case KeyValue::INT: return "INT";
        case KeyValue::LONG: return "LONG";
        case KeyValue::DOUBLE: return "DOUBLE";
        case KeyValue::CHAR: return "CHAR";
        case KeyValue::STRING: return "STRING";
        default: return "UNKNOWN";
    }
}





