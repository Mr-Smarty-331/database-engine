// src/crc32c.cpp

#include "kvstore/crc32c.h"

// This is the header from the library we downloaded via FetchContent.
#include <crc32c/crc32c.h>

namespace kvstore {
namespace crc32c {

uint32_t Extend(uint32_t init_crc, const char* data, size_t n) {
    return ::crc32c::Extend(init_crc, reinterpret_cast<const uint8_t*>(data), n);
}

static const uint32_t kMaskDelta = 0xa282ead8;

uint32_t Mask(uint32_t crc) {
    return ((crc >> 15) | (crc << 17)) + kMaskDelta;
}

uint32_t Unmask(uint32_t masked_crc) {
    // Reverse the transformation.
    uint32_t rotated = masked_crc - kMaskDelta;
    return ((rotated >> 17) | (rotated << 15));
}

} // namespace crc32c
} // namespace kvstore