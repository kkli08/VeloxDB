## SST Files Layout
```
[Internal Node Page (Root)]
[Internal Node Page 1]
[Internal Node Page 2]
...
[Internal Node Page n]
[Leaf Node Page 1]
[Leaf Node Page 2]
[Leaf Node Page 3]
...
[Leaf Node Page m]
[* Clustered Index Page]
[* Bloom Filter Page]
[SST Metadata Page]
```
### `Page::PageSize`
> Page with `PageSize::` **PageSize** (`4KB`, `8KB`)

### `Page::SST_MetaData`
```c++
LeafNode_Begin_Offset
LeafNode_End_offset
FileName
```

### `Page::LeafNodes`
```c++
/*
 *  4kb / 8kb chunk
 *  sorted by key
 */
serialized key-value pair 1 metadata (serialized by protobuf)
serialized key-value pair 2 metadata (serialized by protobuf)
serialized key-value pair 3 metadata (serialized by protobuf)
...
// with padding
```

### `Page::InternalNodes`
```c++
/*
 *  4kb / 8kb chunk
 *  sorted by level
 */
level#0 key-value pair 0 metadata (serialized by protobuf), jump_offset_L1_K0, jump_offset_L1_K1
level#1 key-value pair 1 metadata (serialized by protobuf), jump_offset_L2_K0, jump_offset_L2_K1
level#1 key-value pair 2 metadata (serialized by protobuf), jump_offset_L2_K1, jump_offset_L2_K2
...
// with padding
```

### `Page::BloomFilter`
TBD
### `Page::ClusteredIndex`
TBD