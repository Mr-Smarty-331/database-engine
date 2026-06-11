// tests/memtable_test.cpp

#include "kvstore/comparator.h"
#include "kvstore/db_format.h"
#include "kvstore/memtable.h"
#include <gtest/gtest.h>

TEST(MemTableTest, AddAndGet) {
  // Arrange
  kvstore::BytewiseComparator cmp;
  kvstore::MemTable *memtable = new kvstore::MemTable(&cmp);
  memtable->Ref();

  memtable->Add(kvstore::ValueType::kTypeValue, "key1", "value1");

  std::string value;
  ASSERT_TRUE(memtable->Get("key1", &value));
  EXPECT_EQ(value, "value1");

  ASSERT_FALSE(memtable->Get("key_that_does_not_exist", &value));

  memtable->Unref();
}

TEST(MemTableTest, Tombstone) {
  // Arrange
  kvstore::BytewiseComparator cmp;
  kvstore::MemTable *memtable = new kvstore::MemTable(&cmp);
  memtable->Ref();

  std::string value;

  memtable->Add(kvstore::ValueType::kTypeValue, "key1", "value1");
  ASSERT_TRUE(memtable->Get("key1", &value));

  memtable->Add(kvstore::ValueType::kTypeDeletion, "key1", "");
  ASSERT_FALSE(memtable->Get("key1", &value));

  memtable->Add(kvstore::ValueType::kTypeDeletion, "key2", "");
  ASSERT_FALSE(memtable->Get("key2", &value));

  memtable->Unref();
}

TEST(MemTableTest, Overwrite) {
  // Arrange
  kvstore::BytewiseComparator cmp;
  kvstore::MemTable *memtable = new kvstore::MemTable(&cmp);
  memtable->Ref();
  std::string value;

  // Act & Assert
  memtable->Add(kvstore::ValueType::kTypeValue, "key1", "value1");
  ASSERT_TRUE(memtable->Get("key1", &value));
  EXPECT_EQ(value, "value1");

  memtable->Add(kvstore::ValueType::kTypeValue, "key1", "value2");
  ASSERT_TRUE(memtable->Get("key1", &value));
  EXPECT_EQ(value, "value2");

  memtable->Add(kvstore::ValueType::kTypeDeletion, "key1", "");
  ASSERT_FALSE(memtable->Get("key1", &value));

  memtable->Add(kvstore::ValueType::kTypeValue, "key1", "value3");
  ASSERT_TRUE(memtable->Get("key1", &value));
  EXPECT_EQ(value, "value3");

  memtable->Unref();
}