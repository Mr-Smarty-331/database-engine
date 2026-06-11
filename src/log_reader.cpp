#include "kvstore/log_reader.h"

#include <cstdio>
#include "kvstore/status.h"
#include "kvstore/crc32c.h"
#include "kvstore/coding.h"

namespace kvstore {
namespace log {

Reader::Reader(FILE* file)
    : file_(file),
      backing_store_(),
      eof_(false) {
}

Reader::~Reader() = default;

// The ReadRecord method remains unchanged from the previous step.
// Its logic for handling the return values of ReadPhysicalRecord is already correct.
bool Reader::ReadRecord(Slice* record, std::string* scratch, Status* status) {
    scratch->clear();
    bool in_fragmented_record = false;

    while (true) {
        Slice fragment;
        const RecordType type = ReadPhysicalRecord(&fragment, status);
        
        // --- NEW: Check status immediately after reading a physical record ---
        if (!status->ok()) {
            return false;
        }

        switch (type) {
            case kFullType:
                if (in_fragmented_record) {
                    *status = Status::Corruption("Partial record without end fragment");
                    return false;
                }
                *record = fragment;
                return true;

            case kFirstType:
                if (in_fragmented_record) {
                    *status = Status::Corruption("Another start fragment inside a record");
                    return false;
                }
                scratch->assign(fragment.data(), fragment.size());
                in_fragmented_record = true;
                break;

            case kMiddleType:
                if (!in_fragmented_record) {
                    *status = Status::Corruption("Middle fragment without a start");
                    return false;
                }
                scratch->append(fragment.data(), fragment.size());
                break;

            case kLastType:
                if (!in_fragmented_record) {
                    *status = Status::Corruption("Last fragment without a start");
                    return false;
                }
                scratch->append(fragment.data(), fragment.size());
                *record = Slice(*scratch);
                return true;

            case kZeroType:
                if (in_fragmented_record) {
                    *status = Status::Corruption("EOF in middle of a fragmented record");
                }
                return false; 
            
            default:
                *status = Status::Corruption("Unknown record type");
                return false;
        }
    }
}

// --- MODIFIED: The method signature and implementation are updated ---
RecordType Reader::ReadPhysicalRecord(Slice* result, Status* status) {
    while (true) {
        if (backing_store_.size() < kHeaderSize) {
            if (!eof_) {
                size_t bytes_read = fread(buffer_, 1, kBlockSize, file_);
                if (bytes_read > 0) {
                    backing_store_ = Slice(buffer_, bytes_read);
                } else {
                    eof_ = true;
                    backing_store_.remove_prefix(backing_store_.size());
                }
                continue;
            } else {
                return kZeroType;
            }
        }

        const char* header = backing_store_.data();
        const uint32_t stored_crc = DecodeFixed32(header);
        const uint16_t length = DecodeFixed16(header + 4);
        const unsigned char type_val = header[6];

        if (kHeaderSize + length > backing_store_.size()) {
            // Not enough data for the full payload. Treat as corruption.
            *status = Status::Corruption("Log record payload is truncated");
            return kZeroType;
        }

        // --- NEW: The core validation logic ---
        // 1. Unmask the checksum we read from the header.
        uint32_t expected_crc = crc32c::Unmask(stored_crc);

        // 2. Calculate the actual checksum of the data (type + payload).
        uint32_t actual_crc = crc32c::Value(&header[6], 1); // Checksum for the type byte
        actual_crc = crc32c::Extend(actual_crc, header + kHeaderSize, length); // Extend with payload

        // 3. Compare them.
        if (actual_crc != expected_crc) {
            // Mismatch! The data is corrupt. Report the error and return.
            *status = Status::Corruption("Checksum mismatch in log record");
            return kZeroType;
        }
        
        if (type_val < kFullType || type_val > kLastType) {
            *status = Status::Corruption("Unknown record type found in log");
            return kZeroType;
        }

        *result = Slice(header + kHeaderSize, length);
        backing_store_.remove_prefix(kHeaderSize + length);

        return static_cast<RecordType>(type_val);
    }
}

} // namespace log
} // namespace kvstore