// include/kvstore/skiplist.h

#pragma once

#include <atomic>
#include <random>
#include <cassert>
#include <mutex>

#include "kvstore/slice.h"
#include "kvstore/status.h"
#include "kvstore/comparator.h"

namespace kvstore {

template <typename Comparator>
class SkipList {
public: 
    struct Node {
        const std::string key;
        const std::string value;

        const int height;
        std::atomic<Node*> next_[1]; // Flexible array member, size determined at allocation

        // A factory function to create a node with a specific height.
        static Node* NewNode(const Slice& k, const Slice& v, int h) {
            void* mem = operator new(sizeof(Node) + (h - 1) * sizeof(std::atomic<Node*>));
            return new (mem) Node(k, v, h);
        }

    private:
        // The constructor is private; nodes can only be created via NewNode factory.
        Node(const Slice& k, const Slice& v, int h) 
            : key(k.ToString()), value(v.ToString()), height(h) {}
    };

public:
    explicit SkipList(const Comparator& cmp);

    // Disallow copying and moving.
    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;
    
    ~SkipList();

    void Insert(const Slice& key, const Slice& value);
    bool Get(const Slice& key, std::string* value) const;

    Node* FindGreaterOrEqual(const Slice& key, Node** prev) const;

private:
    int RandomHeight();

    Comparator const comparator_;
    
    Node* const head_;
    
    std::atomic<int> max_height_;
    
    // We can use a simpler random number generator.
    std::default_random_engine rand_generator_;
    std::uniform_int_distribution<int> rand_distribution_;

    mutable std::mutex mutex_;
};

template <typename Comparator>
SkipList<Comparator>::SkipList(const Comparator& cmp)
    : comparator_(cmp),
      head_(Node::NewNode("", "", 12)), // Dummy head node with max height
      max_height_(1),
      rand_generator_(std::random_device{}()),
      rand_distribution_(1, 4) { // kBranching = 4
    // Initialize head node's next pointers to null.
    for (int i = 0; i < 12; ++i) {
        head_->next_[i].store(nullptr);
    }
}

template <typename Comparator>
SkipList<Comparator>::~SkipList() {
    Node* current = head_->next_[0].load();
    while (current != nullptr) {
        Node* next = current->next_[0].load();
        delete current;
        current = next;
    }
    delete head_;
}

template <typename Comparator>
int SkipList<Comparator>::RandomHeight() {
    static const int kMaxHeight = 12;
    int height = 1;
    while (height < kMaxHeight && rand_distribution_(rand_generator_) == 1) {
        height++;
    }
    return height;
}

template <typename Comparator>
typename SkipList<Comparator>::Node* SkipList<Comparator>::FindGreaterOrEqual(const Slice& key, Node** prev) const {
    Node* x = head_;
    int level = max_height_.load(std::memory_order_relaxed) - 1;
    while (true) {
        Node* next = x->next_[level].load(std::memory_order_acquire);
        if (next != nullptr && comparator_.Compare(Slice(next->key), key) < 0) {
            // Keep searching in this list, key is smaller than target
            x = next;
        } else {
            // Found predecessor or end of list
            if (prev != nullptr) {
                prev[level] = x;
            }
            if (level == 0) {
                return next;
            } else {
                // Drop-down to the next level
                level--;
            }
        }
    }
}

template <typename Comparator>
bool SkipList<Comparator>::Get(const Slice& key, std::string* value) const {
    Node* node = FindGreaterOrEqual(key, nullptr);
    if (node != nullptr && comparator_.Compare(Slice(node->key), key) == 0) {
        if (value != nullptr) {
            *value = node->value;
        }
        return true;
    }
    return false;
}

template <typename Comparator>
void SkipList<Comparator>::Insert(const Slice& key, const Slice& value) {
    std::lock_guard<std::mutex> guard(mutex_);

    Node* prev[12]; // kMaxHeight
    Node* x = FindGreaterOrEqual(key, prev);

    assert(x == nullptr || comparator_.Compare(Slice(x->key), key) != 0);

    int height = RandomHeight();
    if (height > max_height_.load(std::memory_order_relaxed)) {
        for (int i = max_height_.load(std::memory_order_relaxed); i < height; i++) {
            prev[i] = head_;
        }
        max_height_.store(height, std::memory_order_relaxed);
    }
    
    Node* new_node = Node::NewNode(key, value, height);

    for (int i = 0; i < height; i++) {
        new_node->next_[i].store(prev[i]->next_[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
        prev[i]->next_[i].store(new_node, std::memory_order_release);
    }
}

} // namespace kvstore
