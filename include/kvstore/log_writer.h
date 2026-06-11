#pragma once
#include <cstdint>
#include "kvstore/status.h"
#include "kvstore/slice.h"

struct _IO_FILE;
typedef struct _IO_FILE FILE;

namespace kvstore{
namespace log{

class Writer{

public : 
    explicit Writer(FILE* dest);

    Writer(const Writer&) = delete;
    Writer& operator=(const Writer&) = delete;

    ~Writer();

    Status AddRecord(const Slice& slice);


private:
    Status EmitPhysicalRecord(RecordType type, const char* ptr, size_t length);

    FILE* dest_;
    int block_offset_; // Current offset in the current block
};

}

}