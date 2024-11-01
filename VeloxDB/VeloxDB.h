//
// Created by Damian Li on 2024-09-20.
//

#ifndef VELOXDB_H
#define VELOXDB_H

#include "KeyValue.h"
#include "Memtable.h"
#include "SstFileManager.h"

class VeloxDB {
    public:
        // constructor
        VeloxDB()
                    : memtable_size(1e4),
    sstFileManager(std::make_shared<SSTFileManager>("defaultDB")), // Create shared SSTFileManager
memtable(std::make_unique<Memtable>(memtable_size, sstFileManager)) {
            // std::cout << "VeloxDB initialized with memtable size: " << memtable_size << std::endl;
        };

        VeloxDB(int memtable_size)
                    : memtable_size(memtable_size),
                      sstFileManager(std::make_shared<SSTFileManager>("defaultDB")),
                      memtable(make_unique<Memtable>(memtable_size, sstFileManager)) {
            // std::cout << "VeloxDB initialized with memtable size: " << memtable_size << std::endl;
        };
        // destructor
        ~VeloxDB() = default;
        void Open(string db_name);
        void Close();


        Memtable* GetMemtable() const {return memtable.get();};
        int SetMemtableSize(int memtable_size);
        void IndexCheck();

        // update with KeyValueWrapper Class
        template<typename K, typename V>
        void Put(K key, V value);

        // GET
        KeyValueWrapper Get(const KeyValueWrapper& KeyValueWrapper);
        // Overloaded Get method (takes a key and uses it for lookup)
        template<typename K>
        KeyValueWrapper Get(K key);

        // SCAN
        set<KeyValueWrapper> Scan(KeyValueWrapper small_key, KeyValueWrapper large_key);
        // Overloaded Scan method (takes two keys and uses them for scanning)
        template<typename K1, typename K2>
        set<KeyValueWrapper> Scan(K1 small_key, K2 large_key);

        // Set buffer pool parameters
        void setBufferPoolParameters(size_t capacity, EvictionPolicy policy);
        void printCacheHit() const {
            std::cout << "Cache hit: " << sstFileManager->getTotalCacheHits() << " times." << std::endl;
        };

    private:
        std::shared_ptr<SSTFileManager> sstFileManager;
        unique_ptr<Memtable> memtable;

        int memtable_size;
        fs::path path; // path for store SSTs
        bool is_open = false;
        // helper function: set memtable_size
        void set_memtable_size(int memtable_size);
        void set_path(fs::path);
        void check_if_open() const {
            if (!is_open) {
                throw runtime_error("Database is not open. Please open the database before performing operations.");
            }
        }
        // Buffer pool parameters
        size_t bufferPoolCapacity;
        EvictionPolicy bufferPoolPolicy;
};


#include "VeloxDB.tpp"
#endif //VELOXDB_H
