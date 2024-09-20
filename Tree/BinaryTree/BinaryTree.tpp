// BinaryTree.tpp

// Templated insert method that creates a KeyValue instance and inserts it into the tree
template<typename K, typename V>
void BinaryTree::insert(K key, V value) {
    KeyValueWrapper kv(key, value);  // Create a KeyValue instance
    insert(root, kv);         // Call the internal insert method
}

// Templated search method that creates a KeyValue instance with the key only and searches the tree
template<typename K>
bool BinaryTree::search(K key) {
    KeyValueWrapper kv(key, 0);  // Create a KeyValue instance with a dummy value
    return search(root, kv);  // Call the internal search method
}