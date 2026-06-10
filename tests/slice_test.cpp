// tests/slice_test.cpp

#include <gtest/gtest.h>
#include "kvstore/slice.h"
#include <string>

// Test suite for the Slice class.
// The first argument to TEST is the test suite name, the second is the test name.
TEST(SliceTest, Constructors) {
    // Arrange: Prepare test data.
    std::string str = "hello";
    const char* c_str = "world";

    // Act & Assert for default constructor.
    kvstore::Slice s1;
    EXPECT_EQ(s1.size(), 0);
    EXPECT_TRUE(s1.empty());

    // Act & Assert for std::string constructor.
    kvstore::Slice s2(str);
    EXPECT_EQ(s2.data(), str.data());
    EXPECT_EQ(s2.size(), 5);
    EXPECT_EQ(s2.ToString(), "hello");

    // Act & Assert for C-style string constructor.
    kvstore::Slice s3(c_str);
    EXPECT_EQ(s3.data(), c_str);
    EXPECT_EQ(s3.size(), 5);
    EXPECT_EQ(s3.ToString(), "world");

    // Act & Assert for pointer and size constructor.
    kvstore::Slice s4(str.data(), 3);
    EXPECT_EQ(s4.ToString(), "hel");
}

TEST(SliceTest, Modifiers) {
    // Arrange
    kvstore::Slice s("abcdef");

    // Act
    s.remove_prefix(3);

    // Assert
    EXPECT_EQ(s.ToString(), "def");
    EXPECT_EQ(s.size(), 3);

    // Act on an edge case: removing more than the size.
    s.remove_prefix(100);
    EXPECT_TRUE(s.empty());
}

TEST(SliceTest, Comparison) {
    // Arrange
    kvstore::Slice s1("apple");
    kvstore::Slice s2("apply");
    kvstore::Slice s3("apple"); // Same content as s1
    kvstore::Slice s4("apple_pie"); // Longer than s1

    // Assert using compare()
    EXPECT_LT(s1.compare(s2), 0); // "apple" < "apply"
    EXPECT_GT(s2.compare(s1), 0); // "apply" > "apple"
    EXPECT_EQ(s1.compare(s3), 0); // "apple" == "apple"
    EXPECT_LT(s1.compare(s4), 0); // "apple" is a prefix of "apple_pie", so it's smaller

    // Assert using overloaded operators
    EXPECT_TRUE(s1 == s3);
    EXPECT_FALSE(s1 == s2);
    EXPECT_TRUE(s1 != s2);
}

TEST(SliceTest, IndexOperator) {
    // Arrange
    kvstore::Slice s("test");

    // Assert
    EXPECT_EQ(s[0], 't');
    EXPECT_EQ(s[1], 'e');
    EXPECT_EQ(s[3], 't');
    // Note: Accessing out of bounds is undefined behavior, so we only test valid indices.
}