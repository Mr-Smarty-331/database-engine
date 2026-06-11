// include/kvstore/log_reader.h

#pragma once

#include <cstdint>
#include "kvstore/slice.h"
#include "kvstore/status.h"
#include "kvstore/log_format.h"

struct _IO_FILE;
typedef struct _IO_FILE FILE;

namespace kvstore {
namespace log {

class Reader {
public:
    explicit Reader(FILE* file);

    // Disallow copying and assigning.
    Reader(const Reader&) = delete;
    Reader& operator=(const Reader&) = delete;

    ~Reader();
    bool ReadRecord(Slice* record, std::string* scratch, Status* status);

private:
    RecordType ReadPhysicalRecord(Slice* result);

    FILE* const file_;
    
    // Buffer for holding one block read from the file.
    char buffer_[kBlockSize];
    
    // A slice that points to the remaining, unparsed portion of buffer_.
    Slice backing_store_;
    
    // True if we have read to the end of the file.
    bool eof_;
};

} // namespace log
} // namespace kvstore