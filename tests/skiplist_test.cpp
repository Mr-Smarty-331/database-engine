// tests/skiplist_test.cpp

#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "kvstore/comparator.h"
#include "kvstore/skiplist.h"

TEST(SkipListTest, InsertAndGet) {
  // Arrange: Create a SkipList with our BytewiseComparator.
  kvstore::BytewiseComparator comparator;
  kvstore::SkipList<kvstore::BytewiseComparator> list(comparator);

  // Act: Insert some key-value pairs.
  list.Insert("key1", "value1");
  list.Insert("key3", "value3");
  list.Insert("key2", "value2");

  // Assert: Use Get() to check if the keys exist.
  std::string value;
  ASSERT_TRUE(list.Get("key1", &value));
  EXPECT_EQ(value, "value1");

  ASSERT_TRUE(list.Get("key2", &value));
  EXPECT_EQ(value, "value2");

  ASSERT_TRUE(list.Get("key3", &value));
  EXPECT_EQ(value, "value3");

  ASSERT_FALSE(list.Get("key4", &value)); // A non-existent key.
}

TEST(SkipListTest, MaintainsSortedOrder) {
  // Arrange
  kvstore::BytewiseComparator comparator;
  kvstore::SkipList<kvstore::BytewiseComparator> list(comparator);

  std::vector<std::string> keys = {"apple", "zebra", "banana", "cherry",
                                   "mango"};
  for (const auto &key : keys) {
    list.Insert(key, "v_" + key);
  }

  // Act & Assert: Traverse the level 0 list and check if keys are sorted.
  // We can do this because we made Node and FindGreaterOrEqual public.

  // Find the first node in the list.
  auto *node = list.FindGreaterOrEqual("", nullptr);

  std::vector<std::string> sorted_keys;
  while (node != nullptr) {
    sorted_keys.push_back(node->key);
    // Move to the next node in the base-level (level 0) linked list.
    node = node->next_[0].load();
  }

  // Assert that the collected keys are in the correct bytewise order.
  ASSERT_EQ(sorted_keys.size(), 5);
  EXPECT_EQ(sorted_keys[0], "apple");
  EXPECT_EQ(sorted_keys[1], "banana");
  EXPECT_EQ(sorted_keys[2], "cherry");
  EXPECT_EQ(sorted_keys[3], "mango");
  EXPECT_EQ(sorted_keys[4], "zebra");
}