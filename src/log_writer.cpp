// src/log_writer.cpp

#include "kvstore/log_writer.h"

#include <cstdint>
#include <cstdio> 
#include "kvstore/coding.h" 
#include "kvstore/log_format.h"
#include "kvstore/crc32c.h"

namespace kvstore {
namespace log {

inline void EncodeFixed32(char* buf, uint32_t value) {
    memcpy(buf, &value, sizeof(value));
}

inline void EncodeFixed16(char* buf, uint16_t value) {
    memcpy(buf, &value, sizeof(value));
}

Writer::Writer(FILE* dest) : dest_(dest), block_offset_(0) {
}

Writer::~Writer() {
}

Status Writer::AddRecord(const Slice& slice) {
    const char* ptr = slice.data();
    size_t left = slice.size();

    Status s;
    bool begin = true; // Is this the first fragment of the record?

    do {
        const int leftover = kBlockSize - block_offset_;
        
        // This assertion ensures our block offset logic is sound.
        assert(leftover >= 0);

        if (leftover < kHeaderSize) {
            if (leftover > 0) {
                // Pad the rest of the block with null characters.
                char pad[kHeaderSize] = {0};
                if (fwrite(pad, 1, leftover, dest_) != (size_t)leftover) {
                    return Status::IOError("Failed to write padding to log");
                }
            }
            // Move to the beginning of the next block.
            block_offset_ = 0;
        }

        // We have a new or partially-filled block. Calculate how much data we can write.
        const size_t avail = kBlockSize - block_offset_ - kHeaderSize;
        const size_t fragment_length = (left < avail) ? left : avail;

        // Determine the type of this physical record fragment.
        RecordType type;
        const bool end = (left == fragment_length); // Is this the last fragment?
        if (begin && end) {
            type = kFullType;
        } else if (begin) {
            type = kFirstType;
        } else if (end) {
            type = kLastType;
        } else {
            type = kMiddleType;
        }

        // Write this fragment to disk.
        s = EmitPhysicalRecord(type, ptr, fragment_length);
        if (!s.ok()) {
            // Stop and return the error if writing fails.
            break;
        }

        // Advance our pointers and counters for the next iteration.
        ptr += fragment_length;
        left -= fragment_length;
        begin = false; // Subsequent fragments are not the first.

    } while (s.ok() && left > 0);

    return s;
}

Status Writer::EmitPhysicalRecord(RecordType t, const char* ptr, size_t n) {
    // The physical record size must fit within a 16-bit integer.
    assert(n <= 0xffff); 
    // The fragment must fit in the current block.
    assert(block_offset_ + kHeaderSize + n <= kBlockSize);

    // 1. Format the header into a buffer.
    char buf[kHeaderSize];
    EncodeFixed16(buf + 4, static_cast<uint16_t>(n)); // Length
    buf[6] = static_cast<char>(t); // Type

    // 2. Compute checksum. The checksum is calculated over the type and the payload.
    uint32_t crc = crc32c::Value(&buf[6], 1);
    crc = crc32c::Extend(crc, ptr, n);
    // Finally, we mask the result for extra robustness.
    crc = crc32c::Mask(crc);
    EncodeFixed32(buf, crc);

    // 3. Write the header to the file.
    if (fwrite(buf, 1, kHeaderSize, dest_) != kHeaderSize) {
        return Status::IOError("Failed to write header to log");
    }

    // 4. Write the payload to the file.
    if (fwrite(ptr, 1, n, dest_) != n) {
        return Status::IOError("Failed to write payload to log");
    }

    // 5. Ensure data is passed to the OS buffer.
    fflush(dest_);

    // 6. Update our position in the block.
    block_offset_ += kHeaderSize + n;

    return Status::OK();
}

} // namespace log
} // namespace kvstore