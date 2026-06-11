#include "kvstore/log_reader.h"

#include <cstdio>
#include "kvstore/coding.h"

namespace kvstore {
namespace log {

Reader::Reader(FILE* file)
    : file_(file),
      backing_store_(),
      eof_(false) {
}

Reader::~Reader() = default;

bool Reader::ReadRecord(Slice* record, std::string* scratch, Status* status) {
    scratch->clear();
    bool in_fragmented_record = false;

    while (true) {
        // Read the next physical record fragment.
        Slice fragment;
        const RecordType type = ReadPhysicalRecord(&fragment);
        
        switch (type) {
            case kFullType:
                if (in_fragmented_record) {
                    // This indicates corruption or an error, as we were in the middle
                    // of a fragmented record but received a full record.
                    *status = Status::Corruption("Partial record without end fragment");
                    return false;
                }
                // A complete record was read.
                *record = fragment;
                return true;

            case kFirstType:
                if (in_fragmented_record) {
                    *status = Status::Corruption("Another start fragment inside a record");
                    return false;
                }
                // Start of a new fragmented record. Append the fragment to scratch.
                scratch->assign(fragment.data(), fragment.size());
                in_fragmented_record = true;
                break;

            case kMiddleType:
                if (!in_fragmented_record) {
                    *status = Status::Corruption("Middle fragment without a start");
                    return false;
                }
                // A middle fragment. Append it to the scratch space.
                scratch->append(fragment.data(), fragment.size());
                break;

            case kLastType:
                if (!in_fragmented_record) {
                    *status = Status::Corruption("Last fragment without a start");
                    return false;
                }
                // The final fragment. Append it, set the record slice, and return.
                scratch->append(fragment.data(), fragment.size());
                *record = Slice(*scratch);
                return true;

            case kZeroType:
                // This indicates EOF or a corruption that ReadPhysicalRecord handled.
                // If we are in a fragmented record, it's an unexpected EOF.
                if (in_fragmented_record) {
                    *status = Status::Corruption("EOF in middle of a fragmented record");
                }
                return false; // Return false for EOF.
            
            default:
                *status = Status::Corruption("Unknown record type");
                return false;
        }
    }
}

RecordType Reader::ReadPhysicalRecord(Slice* result) {
    // Loop until we find a record or hit a hard error.
    while (true) {
        if (backing_store_.size() < kHeaderSize) {
            if (!eof_) {
                // Read the next block from the file into our buffer.
                size_t bytes_read = fread(buffer_, 1, kBlockSize, file_);
                if (bytes_read > 0) {
                    backing_store_ = Slice(buffer_, bytes_read);
                } else {
                    eof_ = true;
                    backing_store_.remove_prefix(backing_store_.size()); // Make buffer empty
                }
                continue; // Retry reading the record from the newly filled buffer.
            } else {
                // We are at the end of the file and have no more data.
                return kZeroType;
            }
        }

        // We have enough data for a header. Parse it.
        const char* header = backing_store_.data();
        const uint16_t length = DecodeFixed16(header + 4);
        const unsigned char type_val = header[6];

        // Check if the entire physical record fits in our current buffer.
        if (kHeaderSize + length > backing_store_.size()) {
            // Not enough data for the full payload. This implies corruption or
            // an incomplete write at the end of the file.
            return kZeroType;
        }
        
        // For now, we only support the known record types.
        if (type_val < kFullType || type_val > kLastType) {
            // Unrecognized record type, indicates corruption.
            return kZeroType;
        }

        // Set the result slice to point to the payload.
        *result = Slice(header + kHeaderSize, length);

        // Advance the backing_store_ past this physical record.
        backing_store_.remove_prefix(kHeaderSize + length);

        return static_cast<RecordType>(type_val);
    }
}

} // namespace log
}