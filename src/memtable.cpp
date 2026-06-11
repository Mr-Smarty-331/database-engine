// src/memtable.cpp

#include "kvstore/memtable.h"

namespace kvstore {

MemTable::MemTable(const Comparator* cmp)
    : comparator_(cmp),       // Initialize our comparator wrapper.
      table_(comparator_),    // Initialize the SkipList with the comparator wrapper.
      refs_(0) {              // Initial reference count is 0. The creator gets one "implicit" ref.
    // When a MemTable is created, its ref count is 0. The creator is responsible
    // for its lifetime until it's shared, at which point they should call Ref().
    // We will see this pattern when we implement the DB class.
}

MemTable::~MemTable() {
    // The destructor for `table_` (our SkipList) will be automatically called,
    // which will deallocate all the nodes. No extra work is needed here.
}

void MemTable::Add(const Slice& key, const Slice& value) {
    // For now, adding to the MemTable is a simple insert into the SkipList.
    table_.Insert(key, value);
}

bool MemTable::Get(const Slice& key, std::string* value) const {
    // The Get operation is a direct delegation to the SkipList's Get method.
    return table_.Get(key, value);
}

} // namespace kvstore
