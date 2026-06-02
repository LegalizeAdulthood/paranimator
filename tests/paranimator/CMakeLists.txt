# SPDX-License-Identifier: GPL-3.0-only
#
function(add_paranimator_test name)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${name}")

    add_test(NAME TestParAnimator.${name}
        COMMAND ${CMAKE_COMMAND}
            "-DPARANIMATOR=$<SHELL_PATH:$<TARGET_FILE:paranimator>>"
            "-DTEST_NAME=${name}"
            "-DTEST_SOURCE_DIR=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}>"
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
            -P "${CMAKE_CURRENT_SOURCE_DIR}/schema_test.cmake"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${name}"
    )
    set_tests_properties(TestParAnimator.${name} PROPERTIES LABELS paranimator)
endfunction()

add_paranimator_test(center-mag)
add_paranimator_schema_test(invalid-schema)
