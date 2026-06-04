// src/main.cpp

#include <iostream>
#include <string>

// Include our new Slice header.
// The path 'kvstore/slice.h' works because of the target_include_directories
// command we added in CMakeLists.txt.
#include "kvstore/slice.h"

void print_slice(const kvstore::Slice& s) {
    std::cout << "\"" << s.ToString() << "\""
              << " (size: " << s.size() << ")" << std::endl;
}

int main() {
    std::cout << "--- Testing Slice ---" << std::endl;

    // Create Slices using different constructors
    kvstore::Slice s1; // Default constructor
    std::string my_key = "my_key";
    kvstore::Slice s2(my_key); // std::string constructor
    kvstore::Slice s3("a c-style string"); // C-style string constructor
    kvstore::Slice s4(my_key.data(), 4); // Pointer and size constructor (view on "my_k")

    std::cout << "s1 (empty): "; print_slice(s1);
    std::cout << "s2 (from std::string): "; print_slice(s2);
    std::cout << "s3 (from C-string): "; print_slice(s3);
    std::cout << "s4 (from ptr+size): "; print_slice(s4);

    // Test comparison
    std::cout << "\n--- Comparing Slices ---" << std::endl;
    if (s2.compare(s4) > 0) {
        std::cout << "\"" << s2.ToString() << "\" is greater than \"" << s4.ToString() << "\"" << std::endl;
    }

    // Test equality
    kvstore::Slice s5("my_key");
    if (s2 == s5) {
        std::cout << "s2 and s5 are equal." << std::endl;
    }

    // Test remove_prefix
    std::cout << "\n--- Testing remove_prefix ---" << std::endl;
    s3.remove_prefix(2); // Should become "c-style string"
    std::cout << "s3 after removing prefix of 2: "; print_slice(s3);

    return 0;
}