# SPDX-License-Identifier: GPL-3.0-only
#
include(GoogleTest)

find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

set(TEST_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/output")
set(TEST_OUTPUT_PAR "frames.par")
set(TEST_OUTPUT_ENTRY "frame-%04d")
set(TEST_OUTPUT_SCRIPT "frames.bat")
set(TEST_VIDEO_MODE "F6")
configure_file("from.par.in" "from.par" @ONLY)
configure_file("to.par.in" "to.par" @ONLY)
configure_file("viewport-catalog.json" "viewport-catalog.json" COPYONLY)
set(TEST_SOURCE_PAR "Mandel_Demo")
set(TEST_TRACK_PARAMETER "center-mag")
set(TEST_TRACK_FROM "-0.5/0/1")
set(TEST_TRACK_TO "-0.5/0/10")
configure_file("config.json.in" "center-mag.json" @ONLY)
set(TEST_SOURCE_PAR "corners")
set(TEST_TRACK_PARAMETER "corners")
set(TEST_TRACK_FROM "-3.570101/-0.0499383/-1.320061/1.320061")
set(TEST_TRACK_TO "-1.8101050271/-1.8099342992/-6.37250230799e-05/6.4320896203e-05")
configure_file("config.json.in" "corners.json" @ONLY)
configure_file("test.h.in" "include/TestParFile/test.h" @ONLY)

add_executable(test-par-file
    TestConfig.cpp
    TestInterpolant.cpp
    TestInterpolator.cpp
    TestJsonSchema.cpp
    TestOutputLayout.cpp
    TestParameter.cpp
    TestParameterCatalog.cpp
    TestParFile.cpp
    TestParSet.cpp
    TestScript.cpp

    e2e-center-mag.json
    from.par.in
    gold-center-mag.par
    invalid-extra-property.json
    invalid-missing-metadata-type.json
    invalid-output-directory-type.json
    paranimator_test.cmake
    to.par.in
    config.json.in
    viewport-catalog.json
    test.h.in
    "${CMAKE_CURRENT_BINARY_DIR}/from.par"
    "${CMAKE_CURRENT_BINARY_DIR}/to.par"
    "${CMAKE_CURRENT_BINARY_DIR}/viewport-catalog.json"
    "${CMAKE_CURRENT_BINARY_DIR}/center-mag.json"
    "${CMAKE_CURRENT_BINARY_DIR}/corners.json"
    "${CMAKE_CURRENT_BINARY_DIR}/include/TestParFile/test.h"
    "${CMAKE_SOURCE_DIR}/schemas/config.schema.json"
    "${CMAKE_SOURCE_DIR}/schemas/parameter-catalog.schema.json"
)
source_group("Template Files" REGULAR_EXPRESSION "^.*\\.in$")
source_group("Data Files" REGULAR_EXPRESSION "^.*\\.(par|json)$")
source_group("CMake Scripts" REGULAR_EXPRESSION "^.*\\.cmake$")
target_include_directories(test-par-file PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(test-par-file PUBLIC ParFile GTest::gtest_main nlohmann_json::nlohmann_json)
target_folder(test-par-file "Tests")

gtest_discover_tests(test-par-file)

add_test(NAME paranimator-center-mag-e2e
    COMMAND ${CMAKE_COMMAND}
        "-DPARANIMATOR=$<SHELL_PATH:$<TARGET_FILE:paranimator>>"
        "-DVALID_CONFIG=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/e2e-center-mag.json>"
        "-DINVALID_CONFIG=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/invalid-extra-property.json>"
        "-DGOLD_PAR=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/gold-center-mag.par>"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/paranimator_test.cmake"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
