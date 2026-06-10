add_executable(kvstore_tests
    slice_test.cpp
    status_test.cpp
)

# Linking our test executable against our main library (kvstore_lib)

target_link_libraries(kvstore_tests PRIVATE 
    kvstore_lib 
    GTest::gtest_main
)

# command from the Google Test CMake for integration - automatically
# finds all TEST() macros in our source files and adds them to CTest
# so they can be run.
include(GoogleTest)
gtest_discover_tests(kvstore_tests)