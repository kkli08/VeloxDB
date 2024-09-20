//
// Created by Damian Li on 2024-08-26.
//

#ifndef MEMTABLE_H
#define MEMTABLE_H
#include "RedBlackTree.h"
#include <filesystem> // C++17 lib
#include "FileManager.h"
namespace fs = std::filesystem;
using namespace std;


/*
 * Handle in-memory operations.
 */
class Memtable {
    public:
        Memtable();
        Memtable(int threshold);
        ~Memtable();
        void set_path(fs::path);
        fs::path get_path();

        // update with KeyValueWrapper Class
        void Scan(KeyValueWrapper small_key, KeyValueWrapper large_key, set<KeyValueWrapper>& res);
        FlushSSTInfo put(const KeyValueWrapper&);
        KeyValueWrapper get(const KeyValueWrapper& kv);

        // helper function
        string generateSstFilename();
        int get_memtableSize() const {return memtable_size;};
        int get_currentSize() const {return current_size;};
        int getSSTFileSize() const {return SST_file_size;};
        void increaseSSTFileSize() {SST_file_size++;};
        RedBlackTree* getTree() const {return tree;};
        // File Manager
        FileManager file_manager;

    private:
        RedBlackTree* tree;
        int memtable_size; // maximum size of memtable
        int current_size = 0;
        fs::path path;
        int SST_file_size = 0;


};


#endif //MEMTABLE_H
