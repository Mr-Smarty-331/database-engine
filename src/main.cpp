// src/main.cpp

#include <iostream>
#include <string>
#include <vector>

// Include both of our new headers
#include "kvstore/slice.h"
#include "kvstore/status.h"

// A dummy function that simulates a database operation.
// It returns a Status object to indicate the outcome.
kvstore::Status find_key(const kvstore::Slice& key) {
    if (key.compare("good_key") == 0) {
        // The operation was successful. Return an OK status.
        return kvstore::Status::OK();
    }
    if (key.compare("missing_key") == 0) {
        // The key was not found. Return a NotFound status with a message.
        return kvstore::Status::NotFound("key not in database");
    }
    // Any other key is considered a data corruption error for this example.
    return kvstore::Status::Corruption("corrupted key format");
}

// A helper function to print a status.
void print_status(const kvstore::Status& s) {
    std::cout << "Status: " << s.ToString() << std::endl;
}

int main() {
    std::cout << "--- Testing Status ---\n" << std::endl;

    // Test Case 1: Successful operation
    std::cout << "Finding 'good_key'..." << std::endl;
    kvstore::Status s1 = find_key("good_key");
    print_status(s1);
    if (s1.ok()) {
        std::cout << "  -> Operation was successful.\n" << std::endl;
    }

    // Test Case 2: Key not found
    std::cout << "Finding 'missing_key'..." << std::endl;
    kvstore::Status s2 = find_key("missing_key");
    print_status(s2);
    if (s2.is_not_found()) {
        std::cout << "  -> Correctly identified as 'Not Found'.\n" << std::endl;
    }

    // Test Case 3: Corrupted data
    std::cout << "Finding 'bad_key'..." << std::endl;
    kvstore::Status s3 = find_key("bad_key");
    print_status(s3);
    if (s3.is_corruption()) {
        std::cout << "  -> Correctly identified as 'Corruption'.\n" << std::endl;
    }

    // Test Case 4: Move semantics
    std::cout << "Testing move constructor..." << std::endl;
    kvstore::Status s4 = kvstore::Status::IOError("cannot read from disk");
    print_status(s4);
    // When s5 is constructed, s4's internal state is moved (not copied).
    kvstore::Status s5 = std::move(s4); 
    std::cout << "s5 (after move from s4): ";
    print_status(s5);
    std::cout << "s4 (after being moved from): ";
    // s4 should now be in a valid but empty (OK) state.
    print_status(s4);

    return 0;
}