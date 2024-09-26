//
// Created by Damian Li on 2024-09-20.
//

#include "VeloxDB.h"
#include <iostream>
#include <string>
#include <filesystem> // C++17 lib

namespace fs = std::filesystem;
  /*
   * void API::Open(string)
   *
   * Open db by name
   */
  void VeloxDB::Open(string db_name) {
    std::cout << "Opening database " << db_name << std::endl;

    if (is_open) {
      throw runtime_error("Database is already open.");
    }

    // Allocate or reallocate memtable and index
    if (!memtable) {
      memtable = make_unique<Memtable>(sstFileManager);
    }

    // Define the path to the database directory
    fs::path db_path = db_name;
    // Check if the directory exists
    if (!fs::exists(db_path)) {
      // Directory does not exist, so create it
      if (fs::create_directory(db_path)) {
        std::cout << "Created database directory: " << db_name << std::endl;
      } else {
        std::cerr << "Failed to create directory: " << db_name << std::endl;
      }
    } else {
      std::cout << "Existed database directory: " << db_name << std::endl;
    }
    // set api attribute fs::path
    set_path(db_path);
    // set flag
    is_open = true;

  }

  /*
   * void API::Close()
   *
   * Close and cleanup the database
   */
  void VeloxDB::Close() {
    check_if_open();
    std::cout << "Closing database " << std::endl;

    // Check if the memtable has any entries before attempting to flush
    if (memtable->get_currentSize() > 0) {
      // The close command should transform whatever is in the current Memtable into an SST
      memtable->flushToDisk();
    } else {
      std::cout << "Memtable is empty. No flush needed." << std::endl;
    }

    // Set the flag to indicate the database is closed
    is_open = false;
  }


  /*
   * KeyValueWrapper API::Get(const KeyValueWrapper&)
   *
   * Return the value of a key, return an empty KeyValueWrapper if the key
   * doesn't exist in memtable or SSTs
   */
  KeyValueWrapper VeloxDB::Get(const KeyValueWrapper& keyValueWrapper) {
    // Check if the database is open
    check_if_open();

    // Attempt to get the value from the memtable
    KeyValueWrapper result = memtable->get(keyValueWrapper);

    if(result.isEmpty()) {
      result = *sstFileManager->search(keyValueWrapper);
    }

    // Return the result (either from memtable or SSTs)
    return result;
  }

  /*
   * set<KeyValueWrapper> API::Scan(KeyValueWrapper, KeyValueWrapper)
   *
   * Search the memtable and the SSTs.
   * step 1:
   *    scan the memtable
   * step 2:
   *    scan the SSTs from oldest to youngest
   */
  set<KeyValueWrapper> VeloxDB::Scan(KeyValueWrapper small_key, KeyValueWrapper large_key) {
    set<KeyValueWrapper> result;

    // step 1:
    // scan the memtable
    memtable->scan(small_key, large_key, result);

    // step 2:
    // scan the SSTs from youngest to oldest
    vector<KeyValueWrapper> Sst_results;
    sstFileManager->scan(small_key, large_key, Sst_results);
    for (const auto & Sst_result : Sst_results) result.insert(Sst_result);

    return result;
  }

  // helper function
  void VeloxDB::set_path(fs::path _path) {
    path = _path;
    memtable->setPath(_path);
    sstFileManager->setPath(_path);

  }


void VeloxDB::setBufferPoolParameters(size_t capacity, EvictionPolicy policy) {
    bufferPoolCapacity = capacity;
    bufferPoolPolicy = policy;

    // Set buffer pool parameters in sstFileManager
    sstFileManager->setBufferPoolParameters(capacity, policy);
  }
