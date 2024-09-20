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
[SST Metadata Page]
```
### `Page::PageSize`
> Page with `PageSize::` **PageSize** (`4KB`, `8KB`)

### `Page::SST_MetaData`

### `Page::BloomFilter`

### `Page::ClusteredIndex`

### `Page::LeafNodes`

### `Page::InternalNode`
