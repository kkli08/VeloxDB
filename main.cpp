//
// Created by Damian Li on 2024-09-20.
//

#include "BTree.h"
#include "KeyValue.h"

int main() {
  BTree tree(3); // B+ Tree of degree 3

  // Insert some key-value pairs with different key types
  tree.insert(KeyValueWrapper(10, "value1"));            // int key
  tree.insert(KeyValueWrapper(20LL, "value2"));          // long key
  tree.insert(KeyValueWrapper(15.5, "value3"));          // double key
  tree.insert(KeyValueWrapper('a', "value4"));           // char key
  tree.insert(KeyValueWrapper(std::string("key5"), 5));  // string key

  // Traverse the tree
  tree.traverse();

  // Search for a key
  KeyValueWrapper searchKey(15.5, ""); // We only need to set the key for searching
  KeyValueWrapper* result = tree.search(searchKey);
  if (result != nullptr) {
    std::cout << "Found: ";
    result->printKeyValue();
  } else {
    std::cout << "Key not found." << std::endl;
  }

  return 0;
}
