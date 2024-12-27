# SPDX-License-Identifier: GPL-3.0-only
#
include(GoogleTest)

find_package(GTest CONFIG REQUIRED)

set(TEST_OUTPUT "frames.par")
set(TEST_VIDEO_MODE "F6")
configure_file("from.par.in" "from.par" @ONLY)
configure_file("to.par.in" "to.par" @ONLY)
set(TEST_INTERPOLATE "center-mag")
set(TEST_FROM_PAR "Mandel_Demo")
set(TEST_TO_PAR "Mandel_Demo")
configure_file("config.json.in" "center-mag.json" @ONLY)
set(TEST_INTERPOLATE "corners")
set(TEST_FROM_PAR "corners")
set(TEST_TO_PAR "corners")
configure_file("config.json.in" "corners.json" @ONLY)
configure_file("test.h.in" "include/TestParFile/test.h" @ONLY)

add_executable(test-par-file
    TestConfig.cpp
    TestInterpolant.cpp
    TestInterpolator.cpp
    TestParameter.cpp
    TestParFile.cpp
    TestParSet.cpp
    TestScript.cpp

    from.par.in
    to.par.in
    config.json.in
    test.h.in
    "${CMAKE_CURRENT_BINARY_DIR}/from.par"
    "${CMAKE_CURRENT_BINARY_DIR}/to.par"
    "${CMAKE_CURRENT_BINARY_DIR}/center-mag.json"
    "${CMAKE_CURRENT_BINARY_DIR}/corners.json"
    "${CMAKE_CURRENT_BINARY_DIR}/include/TestParFile/test.h"
)
source_group("Template Files" REGULAR_EXPRESSION "^.*\\.in$")
source_group("Data Files" REGULAR_EXPRESSION "^.*\\.(par|json)$")
target_include_directories(test-par-file PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(test-par-file PUBLIC ParFile GTest::gtest_main)
target_folder(test-par-file "Tests")

gtest_discover_tests(test-par-file)
