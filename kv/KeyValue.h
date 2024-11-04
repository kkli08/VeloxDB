//
// Created by Damian Li on 2024-09-07.
//
#ifndef KEYVALUEWRAPPER_H
#define KEYVALUEWRAPPER_H

#include "KeyValue.pb.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <chrono> // For time functions

using namespace std;

class KeyValueWrapper {
public:
   // Default constructor
   KeyValueWrapper() = default;

   // Templated constructor that automatically deduces the type
   template<typename K, typename V>
   KeyValueWrapper(K key, V value);

   // Accessor methods
   KeyValue::KeyValueType getKeyType() const { return kv.key_type(); }
   KeyValue::KeyValueType getValueType() const { return kv.value_type(); }

   // Print key-value pair
   void printKeyValue() const;
   std::string keyValueTypeToString(KeyValue::KeyValueType type) const;

   // Comparison operators for keys
   bool operator<(const KeyValueWrapper& other) const;
   bool operator>(const KeyValueWrapper& other) const;
   bool operator<=(const KeyValueWrapper& other) const;
   bool operator>=(const KeyValueWrapper& other) const;
   bool operator==(const KeyValueWrapper& other) const;
   bool operator!=(const KeyValueWrapper& other) const;

   // Serialize and Deserialize methods
   void serialize(std::ostream& os) const;
   static KeyValueWrapper deserialize(std::istream& is);

   bool isEmpty() const;

   // Constructor from a Protobuf KeyValue message
   explicit KeyValueWrapper(const KeyValue& keyValueProto) : kv(keyValueProto), sequenceNumber(0), tombstone(false) {}

   // Convert KeyValueWrapper to Protobuf KeyValue
   KeyValue toProto() const { return kv; }

   // Protobuf-generated KeyValue object
   KeyValue kv;

   // Sequence number for versioning
   uint64_t sequenceNumber = 0;

   // Tombstone flag for deletion
   bool tombstone = false;

   bool isDefault() const {
       // Check if the key is unset
       return kv.key_case() == KeyValue::KEY_NOT_SET && kv.value_case() == KeyValue::VALUE_NOT_SET;
   }

   size_t getSerializedSize() const;

   // Methods to set and get tombstone
   void setTombstone(bool isTombstone);
   void markAsTombstone();
   bool isTombstone() const;

private:
   // Helper to generate sequence number
   void generateSequenceNumber();

   // Helper to deduce and set the key type in Protobuf
   template<typename T>
   void setKey(T key);

   // Helper to deduce and set the value type in Protobuf
   template<typename T>
   void setValue(T value);
};

#include "KeyValue.tpp"

#endif // KEYVALUEWRAPPER_H
