#pragma once

#include <cstddef>
#include <cstdint>

namespace kvstore {
namespace crc32c {

uint32_t Extend(uint32_t init_crc, const char *data, size_t n);

// Compute the CRC-32c checksum of the provided bytes from scratch.
inline uint32_t Value(const char *data, size_t n) { return Extend(0, data, n); }

uint32_t Mask(uint32_t crc);
uint32_t Unmask(uint32_t masked_crc);

} // namespace crc32c
} // namespace kvstore