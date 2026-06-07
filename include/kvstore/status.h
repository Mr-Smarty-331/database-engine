// include/kvstore/status.h

#pragma once

#include <string>
#include "kvstore/slice.h"

namespace kvstore {

class Status {
public:
    // Create a success status.
    Status() noexcept : state_(nullptr) {}
    
    // Destructor: cleans up the dynamically allocated state for error statuses.
    ~Status() { delete[] state_; }

    // --- Copy and Move Semantics ---
    // The rule of 5: if you have a destructor, you likely need copy/move constructors
    // and copy/move assignment operators to handle ownership correctly.

    // Copy constructor: creates a deep copy of the state.
    Status(const Status& rhs);
    // Copy assignment: assigns a deep copy of the state.
    Status& operator=(const Status& rhs);

    // Move constructor: "steals" the state from the other object.
    Status(Status&& rhs) noexcept;
    // Move assignment: "steals" the state from the other object.
    Status& operator=(Status&& rhs) noexcept;

    // --- Factory Methods for Creating Status Objects ---
    // These static methods are the only way to create non-OK (error) statuses.
    // This makes the calling code very clear, e.g., 'return Status::NotFound(...)'.

    static Status OK() { return Status(); }
    static Status NotFound(const Slice& msg) { return Status(kNotFound, msg); }
    static Status Corruption(const Slice& msg) { return Status(kCorruption, msg); }
    static Status IOError(const Slice& msg) { return Status(kIOError, msg); }

    // --- Status Checking Methods ---
    bool ok() const { return (state_ == nullptr); }
    bool is_not_found() const { return code() == kNotFound; }
    bool is_corruption() const { return code() == kCorruption; }
    bool is_io_error() const { return code() == kIOError; }

    // Returns a human-readable string representation of the status.
    // e.g., "NotFound: key not in database"
    std::string ToString() const;

private:
    // An enum for the different kinds of errors.
    enum Code {
        kOk = 0,
        kNotFound = 1,
        kCorruption = 2,
        kIOError = 3,
    };

    // The state_ pointer.
    // For an OK status, state_ is nullptr.
    // For an error status, state_ points to a dynamically allocated buffer.
    // The buffer has the following format:
    //    state_[0..3] == length of message
    //    state_[4]    == error code
    //    state_[5..]  == message data
    const char* state_;

    // Private constructor used by the factory methods to create error statuses.
    Status(Code code, const Slice& msg);

    // Helper to get the code from the state.
    Code code() const {
        return (state_ == nullptr) ? kOk : static_cast<Code>(state_[4]);
    }

    // Helper to copy the state buffer.
    static const char* CopyState(const char* s);
};

} // namespace kvstore