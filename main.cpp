//
// Created by Damian Li on 2024-09-20.
//

#include "DiskBTree.h"
#include "KeyValue.h"
#include "Memtable.h"

int main() {
    Memtable memtable(5); // Memtable threshold set to 5 for testing

    // Insert key-value pairs
    memtable.put(KeyValueWrapper(10, "value1"));
    memtable.put(KeyValueWrapper(20, "value2"));
    memtable.put(KeyValueWrapper(5, "value3"));
    memtable.put(KeyValueWrapper(6, "value4"));
    memtable.put(KeyValueWrapper(12, "value5"));

    // The memtable should flush to disk here due to threshold

    // memtable.put(KeyValueWrapper(30, "value6"));

    // Get a value
    // KeyValueWrapper result = memtable.get(KeyValueWrapper(12, ""));
    // if (!result.isEmpty()) {
    //     std::cout << "Found: ";
    //     result.printKeyValue();
    // } else {
    //     std::cout << "Key not found." << std::endl;
    // }

    // // Scan a range
    // std::set<KeyValueWrapper> scanResult;
    // memtable.Scan(KeyValueWrapper(5, ""), KeyValueWrapper(20, ""), scanResult);
    // for (const auto& kv : scanResult) {
    //     kv.printKeyValue();
    // }

    return 0;
}
