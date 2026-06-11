// src/main.cpp

#include <iostream>
#include <string>
#include <vector>
#include <cstdio>

#include "kvstore/slice.h"
#include "kvstore/status.h"
#include "kvstore/log_writer.h"
#include "kvstore/log_reader.h"

void test_log_corruption() {
    std::cout << "\n--- Testing Log Corruption Detection ---\n";
    const char* filename = "corruption_test.log";

    // --- Phase 1: Write a clean log file ---
    FILE* log_file = fopen(filename, "wb");
    if (!log_file) {
        std::cerr << "Failed to open log file for writing." << std::endl;
        return;
    }
    {
        kvstore::log::Writer writer(log_file);
        writer.AddRecord("record 1");
        writer.AddRecord("record 2");
        writer.AddRecord("record 3");
    }
    fclose(log_file);
    std::cout << "Successfully wrote 3 records to " << filename << std::endl;

    // --- Phase 2: Manually corrupt the file ---
    // We will open the file in read/write mode and flip a bit in the second record's payload.
    log_file = fopen(filename, "r+b"); // Open for reading and writing
    if (!log_file) {
        std::cerr << "Failed to open log file for corruption." << std::endl;
        return;
    }
    // Seek to a byte inside the payload of the second record and change it.
    // Record 1: header(7) + payload(8) = 15 bytes.
    // Record 2 header starts at 15. Payload starts at 15 + 7 = 22.
    // Let's corrupt the byte at offset 25.
    fseek(log_file, 25, SEEK_SET);
    char corrupted_byte = 'X';
    fwrite(&corrupted_byte, 1, 1, log_file);
    fclose(log_file);
    std::cout << "Manually corrupted a byte in the second record." << std::endl;

    // --- Phase 3: Try to read the corrupted log file ---
    log_file = fopen(filename, "rb");
    if (!log_file) {
        std::cerr << "Failed to open log file for reading." << std::endl;
        return;
    }

    kvstore::log::Reader reader(log_file);
    kvstore::Slice record;
    std::string scratch;
    kvstore::Status status;
    int records_read = 0;

    std::cout << "Reading back the log file..." << std::endl;
    while(reader.ReadRecord(&record, &scratch, &status)) {
        records_read++;
        std::cout << "  Read record: " << record.ToString() << std::endl;
    }

    fclose(log_file);

    // --- Phase 4: Assert the outcome ---
    std::cout << "Finished reading. Total records read successfully: " << records_read << std::endl;
    if (!status.ok()) {
        std::cout << "Reader stopped with an error as expected: " << status.ToString() << std::endl;
        if (status.is_corruption() && records_read == 1) {
            std::cout << "SUCCESS: The corruption was detected correctly after reading the first valid record." << std::endl;
        } else {
            std::cout << "FAILURE: The outcome was not as expected." << std::endl;
        }
    } else {
        std::cout << "FAILURE: The reader finished without reporting a corruption error." << std::endl;
    }
}

int main() {
    // You can keep your old tests for Slice and Status, or just run this one.
    test_log_corruption();
    return 0;
}