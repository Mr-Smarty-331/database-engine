// src/status.cpp

#include <cassert>
#include "kvstore/status.h"

namespace kvstore {

// --- Copy and Move Implementations ---

// Copy constructor implementation.
Status::Status(const Status& rhs) {
    // If rhs.state_ is nullptr, our state_ will be nullptr (from default initialization).
    // Otherwise, we create a deep copy.
    state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
}

// Copy assignment implementation.
Status& Status::operator=(const Status& rhs) {
    // Self-assignment check.
    if (this != &rhs) {
        // First, release our own state.
        delete[] state_;
        // Then, copy the other state.
        state_ = (rhs.state_ == nullptr) ? nullptr : CopyState(rhs.state_);
    }
    return *this;
}

// Move constructor implementation.
Status::Status(Status&& rhs) noexcept : state_(rhs.state_) {
    // After "stealing" the pointer, we set the source object's pointer
    // to nullptr. This prevents the source's destructor from freeing the memory
    // that we now own.
    rhs.state_ = nullptr;
}

// Move assignment implementation.
Status& Status::operator=(Status&& rhs) noexcept {
    // Self-assignment check, although less likely with rvalues.
    if (this != &rhs) {
        // Release our current state.
        delete[] state_;
        // Steal the state from the source.
        state_ = rhs.state_;
        // Nullify the source's pointer.
        rhs.state_ = nullptr;
    }
    return *this;
}

// Private constructor implementation for creating error statuses.
Status::Status(Code code, const Slice& msg) {
    assert(code != kOk);
    const uint32_t len = static_cast<uint32_t>(msg.size());
    // Total size: 4 bytes for length, 1 for code, and the message length.
    const uint32_t total_size = len + 5; 
    char* result = new char[total_size];
    
    // Encode the length at the beginning of the buffer.
    memcpy(result, &len, sizeof(len));
    // Store the error code.
    result[4] = static_cast<char>(code);
    // Copy the message payload.
    memcpy(result + 5, msg.data(), len);

    state_ = result;
}

// Helper to create a deep copy of a state buffer.
const char* Status::CopyState(const char* s) {
    // Get the length from the first 4 bytes.
    uint32_t len;
    memcpy(&len, s, sizeof(len));
    // Total size = 4 (length) + 1 (code) + message_length
    const uint32_t total_size = len + 5;
    char* result = new char[total_size];
    memcpy(result, s, total_size);
    return result;
}

std::string Status::ToString() const {
    if (ok()) {
        return "OK";
    }

    char tmp[30];
    const char* type;
    switch (code()) {
        case kNotFound:
            type = "NotFound: ";
            break;
        case kCorruption:
            type = "Corruption: ";
            break;
        case kIOError:
            type = "IOError: ";
            break;
        default:
            snprintf(tmp, sizeof(tmp), "Unknown code(%d): ", static_cast<int>(code()));
            type = tmp;
            break;
    }
    
    std::string result(type);
    uint32_t len;
    memcpy(&len, state_, sizeof(len));
    result.append(state_ + 5, len); // Append the message part of the buffer
    return result;
}

} // namespace kvstore
