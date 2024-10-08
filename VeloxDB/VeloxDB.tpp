/*
* template function to put key-value pair
 * K = key type, V = value type
 */
template<typename K, typename V>
void VeloxDB::Put(K key, V value) {
    check_if_open();

    // Create a KeyValueWrapper instance and insert it into the memtable
    KeyValueWrapper kvWrapper(key, value);
    memtable->put(kvWrapper);
}

// Overloaded Get method to simplify retrieval by passing a key directly
template<typename K>
KeyValueWrapper VeloxDB::Get(K key) {
    check_if_open();
    // Convert the key to a KeyValueWrapper and call the existing Get method
    KeyValueWrapper kvWrapper(key, "");
    return Get(kvWrapper);
}

// Overloaded Scan method to simplify scanning by passing two keys directly
template<typename K1, typename K2>
set<KeyValueWrapper> VeloxDB::Scan(K1 small_key, K2 large_key) {
    // Convert the keys to KeyValueWrapper and call the existing Scan method
    KeyValueWrapper kvSmallKey(small_key, "");
    KeyValueWrapper kvLargeKey(large_key, "");
    return Scan(kvSmallKey, kvLargeKey);
}