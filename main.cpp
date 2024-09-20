//
// Created by Damian Li on 2024-09-20.
//

#include "BTree.h"
#include "KeyValue.h"
int main() {
  BTree tree(3); // B+ Tree of order 3

  // Insert some key-value pairs
  tree.insert(KeyValueWrapper("key1", "value1"));
  tree.insert(KeyValueWrapper("key2", "value2"));
  tree.insert(KeyValueWrapper("key3", "value3"));
  tree.insert(KeyValueWrapper("key4", "value4"));
  tree.insert(KeyValueWrapper("key5", "value5"));

  // Traverse the tree
  tree.traverse();

  // Search for a key
  KeyValueWrapper* result = tree.search("key3");
  if (result != nullptr) {
    std::cout << "Found: ";
    result->printKeyValue();
  } else {
    std::cout << "Key not found." << std::endl;
  }

  return 0;
}
