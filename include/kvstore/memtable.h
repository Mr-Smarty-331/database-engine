// include/kvstore/memtable.h

#pragma once

#include <atomic>
#include <string>

#include "kvstore/comparator.h"
#include "kvstore/skiplist.h"
#include "kvstore/slice.h"

namespace kvstore {

class MemTable {
public:
    // A MemTable is created with a specific comparator to order its keys.
    explicit MemTable(const Comparator* cmp);

    // We explicitly disallow copying the MemTable.
    MemTable(const MemTable&) = delete;
    MemTable& operator=(const MemTable&) = delete;

    void Ref() {
        refs_.fetch_add(1, std::memory_order_relaxed);
    }

    void Unref() {
        if (refs_.fetch_sub(1, std::memory_order_release) - 1 == 0) {
            std::atomic_thread_fence(std::memory_order_acquire);
            delete this;
        }
    }

    // Add a key-value pair to the MemTable.
    // In the next task, we will enhance this to handle different types of operations.
    void Add(const Slice& key, const Slice& value);

    bool Get(const Slice& key, std::string* value) const;

private:
    ~MemTable();

    struct KeyComparator {
        const Comparator* comparator;
        explicit KeyComparator(const Comparator* c) : comparator(c) {}
        int operator()(const Slice& a, const Slice& b) const {
            return comparator->Compare(a, b);
        }
        int Compare(const Slice& a, const Slice& b) const {
            return comparator->Compare(a, b);
        }
    };

    KeyComparator comparator_;
    SkipList<KeyComparator> table_;
    std::atomic<int> refs_;
};

} // namespace kvstore
