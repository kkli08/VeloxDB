## VeloxDB

VeloxDB is a persistent key-value store database library. It designed to store 
key-value pairs and allow efficient retrieval based on the key. This system is 
inspired by modern databases like [LevelDB](https://github.com/google/leveldb) 
and [RocksDB](https://github.com/facebook/rocksdb), and supports multiple data 
types using C++ Templates and Protocol Buffers.



## SST Files Layout
```
[Root Node Page]
[Internal Node Page 1]
[Internal Node Page 2]
...
[Internal Node Page n]
[Leaf Node Page 1]
[Leaf Node Page 2]
[Leaf Node Page 3]
...
[Leaf Node Page m]
[Clustered Index Page]
[Bloom Filter Page]
```
### Page
Page with `PageSize::` **PageSize** (`4KB`, `8KB`)
### `Root` Block

### `Internal` Block

### `Leaf` Block

### `Clustered Index` Block

### `Bloom Filter` Block