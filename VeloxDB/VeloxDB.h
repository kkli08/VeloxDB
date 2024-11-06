//
// Created by Damian Li on 2024-09-20.
//

// VeloxDB.h
#ifndef VELOXDB_H
#define VELOXDB_H

#include "KeyValue.h"
#include "LSMTree.h"
#include <memory>
#include <filesystem>

class VeloxDB {
public:
    // Constructors
    VeloxDB();
    VeloxDB(int memtable_size);

    // Destructor
    ~VeloxDB();

    void Open(const std::string& db_name);
    void Close();

    // API methods
    template<typename K, typename V>
    void Put(K key, V value);

    // GET
    KeyValueWrapper Get(const KeyValueWrapper& keyValueWrapper);
    // Overloaded Get method (takes a key and uses it for lookup)
    template<typename K>
    KeyValueWrapper Get(K key);

    // SCAN
    std::vector<KeyValueWrapper> Scan(const KeyValueWrapper& small_key, const KeyValueWrapper& large_key);
    // Overloaded Scan method (takes two keys and uses them for scanning)
    template<typename K1, typename K2>
    std::vector<KeyValueWrapper> Scan(K1 small_key, K2 large_key);

    // DELETE
    void Delete(KeyValueWrapper& keyValueWrapper);
    // Overloaded Delete method (takes a key and uses it for lookup)
    template<typename K>
    void Delete(K key);

    // UPDATE
    int Update(const KeyValueWrapper& keyValueWrapper);
    // Overloaded Update method (takes a key and uses it for lookup)
    template<typename K, typename V>
    int Update(K key, V value);


    // Set buffer pool parameters
    void setBufferPoolParameters(size_t capacity, EvictionPolicy policy);
    void printCacheHit() const;

private:
    std::unique_ptr<LSMTree> lsmTree;

    int memtable_size;
    std::filesystem::path path; // Path for storing SSTs
    bool is_open = false;

    void set_path(const std::filesystem::path& db_path);
    void check_if_open() const {
        if (!is_open) {
            throw std::runtime_error("Database is not open. Please open the database before performing operations.");
        }
    }
    // Buffer pool parameters
    size_t bufferPoolCapacity;
    EvictionPolicy bufferPoolPolicy;
};

#include "VeloxDB.tpp"
#endif // VELOXDB_H

