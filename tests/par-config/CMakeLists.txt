# SPDX-License-Identifier: GPL-3.0-only
#
function(add_par_config_test name mode entry frames)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${name}")

    add_test(NAME TestParConfig.${name}
        COMMAND ${CMAKE_COMMAND}
            "-DPAR_CONFIG=$<SHELL_PATH:$<TARGET_FILE:par-config>>"
            "-DPARANIMATOR=$<SHELL_PATH:$<TARGET_FILE:paranimator>>"
            "-DTEST_MODE=${mode}"
            "-DTEST_ENTRY=${entry}"
            "-DTEST_FRAMES=${frames}"
            "-DTEST_SOURCE_DIR=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}>"
            "-DCORE_CATALOG=$<SHELL_PATH:${CMAKE_SOURCE_DIR}/data/core-catalog.json>"
            "-DGOLD_JSON=$<SHELL_PATH:${CMAKE_CURRENT_SOURCE_DIR}/gold-${name}.json>"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/par-config_test.cmake"
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${name}"
    )
    set_tests_properties(TestParConfig.${name} PROPERTIES LABELS par-config)
endfunction()

add_par_config_test(center-mag center-mag Mandel_Demo 3)
add_par_config_test(corners corners Corners_Demo 3)
