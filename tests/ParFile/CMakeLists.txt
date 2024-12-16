include(GoogleTest)

find_package(GTest CONFIG REQUIRED)

add_executable(test-par-file test.cpp)
target_link_libraries(test-par-file PUBLIC ParFile GTest::gtest_main)
target_folder(test-par-file "Tests")

gtest_discover_tests(test-par-file)
