# SPDX-License-Identifier: GPL-3.0-only
#
function(add_paranimator_test name)
    set(options)
    set(one_value_args CATALOG FRAMES)
    set(multi_value_args)
    cmake_parse_arguments(TEST
        "${options}"
        "${one_value_args}"
        "${multi_value_args}"
        ${ARGN}
    )
    if(NOT TEST_FRAMES)
        set(TEST_FRAMES 3)
    endif()
    if(TEST_CATALOG)
        set(TEST_CATALOG_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${TEST_CATALOG}")
    else()
        set(TEST_CATALOG_PATH "${CMAKE_SOURCE_DIR}/data/core-catalog.json")
    endif()
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${name}")

    add_test(NAME TestParAnimator.${name}
        COMMAND ${CMAKE_COMMAND}
            "-DPARANIMATOR=$<SHELL_PATH:$<TARGET_FILE:paranimator>>"
            "-DTEST_NAME=${name}"
            "-DTEST_FRAMES=${TEST_FRAMES}"
            "-DTEST_SOURCE_DIR=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}>"
            "-DCORE_CATALOG=$<SHELL_PATH:${TEST_CATALOG_PATH}>"
            "-DGOLD_PAR=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/gold-${name}.par>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/paranimator_test.cmake"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${name}"
    )
    set_tests_properties(TestParAnimator.${name} PROPERTIES LABELS paranimator)
endfunction()

function(add_paranimator_schema_test name)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${name}")

    add_test(NAME TestParAnimator.${name}
        COMMAND ${CMAKE_COMMAND}
            "-DPARANIMATOR=$<SHELL_PATH:$<TARGET_FILE:paranimator>>"
            "-DTEST_NAME=${name}"
            "-DTEST_SOURCE_DIR=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}>"
            "-DCORE_CATALOG=$<SHELL_PATH:${CMAKE_SOURCE_DIR}/data/core-catalog.json>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/schema_test.cmake"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${name}"
    )
    set_tests_properties(TestParAnimator.${name} PROPERTIES LABELS paranimator)
endfunction()

add_paranimator_test(center-mag)
add_paranimator_test(corners)
add_paranimator_test(maxiter)
add_paranimator_test(bailout)
add_paranimator_test(multi-track)
add_paranimator_test(maxiter-base CATALOG maxiter-base-catalog.json FRAMES 4)
add_paranimator_test(maxiter-omit CATALOG maxiter-omit-catalog.json FRAMES 4)
add_paranimator_test(maxiter-cycle CATALOG maxiter-cycle-catalog.json FRAMES 5)
add_paranimator_test(maxiter-ping-pong CATALOG maxiter-ping-pong-catalog.json FRAMES 5)
add_paranimator_test(maxiter-hold FRAMES 4)
add_paranimator_test(maxiter-step FRAMES 4)
add_paranimator_test(inside-hold)
add_paranimator_schema_test(invalid-schema)
