// src/memtable.cpp

#include "kvstore/memtable.h"

namespace kvstore {

MemTable::MemTable(const Comparator *cmp)
    : comparator_(cmp), // Initialize our comparator wrapper.
      table_(
          comparator_), // Initialize the SkipList with the comparator wrapper.
      refs_(0) {        // Initial reference count is 0. The creator gets one
                        // "implicit" ref.
  // When a MemTable is created, its ref count is 0. The creator is responsible
  // for its lifetime until it's shared, at which point they should call Ref().
  // We will see this pattern when we implement the DB class.
}

MemTable::~MemTable() {
  // The destructor for `table_` (our SkipList) will be automatically called,
  // which will deallocate all the nodes. No extra work is needed here.
}

void MemTable::Add(ValueType type, const Slice &key, const Slice &value) {
  // We need to encode the ValueType and the user's value into a single
  // string that will be stored in the SkipList.
  // Format: [type (1 byte)] + [user_value (variable length)]
  std::string encoded_value;
  encoded_value.push_back(static_cast<char>(type));
  encoded_value.append(value.data(), value.size());

  // The SkipList::Insert method will internally create a new Node,
  // which copies the key and our encoded_value into its own std::string
  // members.
  table_.Insert(key, Slice(encoded_value));
}

bool MemTable::Get(const Slice &key, std::string *value) const {
  std::string internal_value;
  if (table_.Get(key, &internal_value)) {
    // An entry was found. We must now decode its type.
    // An empty internal_value would be an error, but we check defensively.
    if (internal_value.empty()) {
      return false;
    }

    ValueType type = static_cast<ValueType>(internal_value[0]);
    if (type == ValueType::kTypeValue) {
      // This is a normal value. Extract it and return true.
      *value = internal_value.substr(1);
      return true;
    }
  }
  // If we are here, either the key was not found at all, or it was a tombstone.
  // In both cases, the key is considered "not found" from the user's
  // perspective.
  return false;
}

} // namespace kvstore
