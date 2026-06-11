#pragma once

namespace kvstore{

namespace log{

enum RecordType {
    kZeroType = 0, // invalid record
    kFullType = 1, // contains entire data
    kFirstType = 2, // first fragment of data
    kMiddleType = 3, // middle fragments of data
    kLastType = 4 // last fragment of data
};

static const int kHeaderSize = 4 + 2 + 1;
static const int kBlockSize = 32 * 1024;


} // namespace log
} // namespace kvstore