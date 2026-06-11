
#include "kvstore/comparator.h"
#include "kvstore/memtable.h"
#include <gtest/gtest.h>

TEST(MemTableTest, AddAndGet) {
  // Arrange: Create a MemTable with the standard BytewiseComparator.
  kvstore::BytewiseComparator cmp;
  kvstore::MemTable* memtable = new kvstore::MemTable(&cmp);
  // The creator gets a reference. Let's model that by calling Ref().
  memtable->Ref();

  // Act: Add some key-value pairs.
  memtable->Add("key1", "value1");
  memtable->Add("key3", "value3");
  memtable->Add("key2", "value2");

  // Assert: Check if the keys can be retrieved correctly.
  std::string value;
  ASSERT_TRUE(memtable->Get("key1", &value));
  EXPECT_EQ(value, "value1");

  ASSERT_TRUE(memtable->Get("key2", &value));
  EXPECT_EQ(value, "value2");

  ASSERT_TRUE(memtable->Get("key3", &value));
  EXPECT_EQ(value, "value3");

  // Check for a non-existent key.
  ASSERT_FALSE(memtable->Get("key4", &value));

  // Release our reference. This should cause the MemTable to be deleted.
  memtable->Unref();
}
