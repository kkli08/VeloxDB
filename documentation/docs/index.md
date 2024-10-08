# VeloxDB
![Unit Tests](https://github.com/kkli08/VeloxDB/actions/workflows/cmake-unit-tests-multi-platform.yml/badge.svg)

VeloxDB is a persistent key-value store database library. It designed to store
key-value pairs and allow efficient retrieval based on the key. This system is
inspired by modern databases like [LevelDB](https://github.com/google/leveldb)
and [RocksDB](https://github.com/facebook/rocksdb), and supports multiple data
types using C++ Templates and Protocol Buffers.


## Supported Data Types
> 2024-09-12 Restructure with `Protobuf`
> Using **Protocol Buffer** for data serialization

    syntax = "proto3";

    message KeyValue {

      oneof key {
        int32 int_key = 1;
        int64 long_key = 2;
        double double_key = 3;
        string string_key = 4;
        string char_key = 5;
      }
    
      oneof value {
        int32 int_value = 6;
        int64 long_value = 7;
        double double_value = 8;
        string string_value = 9;
        string char_value = 10;
      }
    
      enum KeyValueType {
        INT = 0;
        LONG = 1;
        DOUBLE = 2;
        CHAR = 3;
        STRING = 4;

      }
    
      KeyValueType key_type = 11;
      KeyValueType value_type = 12;
    }

> 2024-09-09 Support Template<typename K, typename V>
```c++
enum KeyValueType { INT, LONG, DOUBLE, CHAR, STRING };
```

> 2024-08-28 Support <int_64, int_64>
>




[//]: # (### Dataflow Diagram)

[//]: # (![DFD]&#40;/img/dfd/kvdb_lv0_v2.0.jpg&#41;)

[//]: # ()
[//]: # (### UML)

[//]: # (![UML]&#40;img/uml/kvdb_s2_uml_v2.1.jpg&#41;)


## Supported Platforms
The KV-Store system has been tested across multiple platforms and compilers. Below is the current support status:

| Platform     | Compiler       | Status |
|--------------|----------------|--------|
| MacOS ARM64  | GCC            | ✅     |
| Ubuntu ARM64 | GCC            | ✅     |
| Ubuntu ARM64 | Clang          | ✅     |
| Windows x86  | MSVC (cl)      | ✅     |


## Legacy Repo

_[KvDB](https://github.com/kkli08/KvDB)_

_[KV-Store](https://github.com/kkli08/KV-Store)_