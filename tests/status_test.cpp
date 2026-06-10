// tests/status_test.cpp

#include <gtest/gtest.h>
#include "kvstore/status.h"

// Test suite for the Status class.
TEST(StatusTest, OkStatus) {
    // Arrange & Act
    kvstore::Status s = kvstore::Status::OK();

    // Assert
    EXPECT_TRUE(s.ok());
    EXPECT_EQ(s.ToString(), "OK");
}

TEST(StatusTest, ErrorStatus) {
    // Arrange & Act
    kvstore::Status s_not_found = kvstore::Status::NotFound("missing key");
    kvstore::Status s_corruption = kvstore::Status::Corruption("bad checksum");
    kvstore::Status s_io = kvstore::Status::IOError("disk read failed");

    // Assert for NotFound
    EXPECT_FALSE(s_not_found.ok());
    EXPECT_TRUE(s_not_found.is_not_found());
    EXPECT_FALSE(s_not_found.is_corruption());
    EXPECT_EQ(s_not_found.ToString(), "NotFound: missing key");

    // Assert for Corruption
    EXPECT_FALSE(s_corruption.ok());
    EXPECT_TRUE(s_corruption.is_corruption());
    EXPECT_EQ(s_corruption.ToString(), "Corruption: bad checksum");

    // Assert for IOError
    EXPECT_FALSE(s_io.ok());
    EXPECT_TRUE(s_io.is_io_error());
    EXPECT_EQ(s_io.ToString(), "IOError: disk read failed");
}

TEST(StatusTest, CopyAndMove) {
    // --- Test Move Constructor ---
    // Arrange
    kvstore::Status a = kvstore::Status::IOError("move me");
    
    // Act
    // This invokes the move constructor. 'b' should take ownership of the state from 'a'.
    kvstore::Status b = std::move(a);

    // Assert
    // 'a' should be in a valid but empty (OK) state after being moved from.
    EXPECT_TRUE(a.ok());
    // 'b' should now hold the error state.
    EXPECT_TRUE(b.is_io_error());
    EXPECT_EQ(b.ToString(), "IOError: move me");

    // --- Test Move Assignment ---
    // Arrange
    kvstore::Status c = kvstore::Status::Corruption("move assign me");
    kvstore::Status d = kvstore::Status::OK();

    // Act
    // This invokes the move assignment operator.
    d = std::move(c);

    // Assert
    // 'c' is now OK.
    EXPECT_TRUE(c.ok());
    // 'd' now holds the error.
    EXPECT_TRUE(d.is_corruption());
    EXPECT_EQ(d.ToString(), "Corruption: move assign me");

    // --- Test Copy Constructor ---
    // Arrange
    kvstore::Status e = kvstore::Status::NotFound("copy me");

    // Act
    // This invokes the copy constructor. 'f' gets a deep copy of the state.
    kvstore::Status f = e;

    // Assert
    // 'e' should be unaffected by the copy.
    EXPECT_TRUE(e.is_not_found());
    EXPECT_EQ(e.ToString(), "NotFound: copy me");
    // 'f' should be an identical, independent copy.
    EXPECT_TRUE(f.is_not_found());
    EXPECT_EQ(f.ToString(), "NotFound: copy me");
}