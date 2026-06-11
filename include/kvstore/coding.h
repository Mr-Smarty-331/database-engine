// include/kvstore/coding.h

#pragma once

#include <cstdint>
#include <cstring> // for memcpy

namespace kvstore {

// --- Encoding Functions ---

inline void EncodeFixed32(char* buf, uint32_t value) {
    memcpy(buf, &value, sizeof(value));
}

inline void EncodeFixed16(char* buf, uint16_t value) {
    memcpy(buf, &value, sizeof(value));
}

// --- Decoding Functions ---

inline uint32_t DecodeFixed32(const char* ptr) {
    uint32_t result;
    memcpy(&result, ptr, sizeof(result));
    return result;
}

inline uint16_t DecodeFixed16(const char* ptr) {
    uint16_t result;
    memcpy(&result, ptr, sizeof(result));
    return result;
}

} // namespace kvstore