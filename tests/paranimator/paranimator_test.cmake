# SPDX-License-Identifier: GPL-3.0-only
#
foreach(name
        PARANIMATOR
        TEST_NAME
        TEST_SOURCE_DIR
        COLORING_CATALOG
        CORE_CATALOG
        GOLD_PAR
        GOLD_SCRIPT
        GOLD_COMPOSE_SCRIPT)
    if(NOT DEFINED ${name})
        message(FATAL_ERROR "Missing required variable: ${name}")
    endif()
endforeach()

set(input_directory "input")
set(output_directory "output")
set(generated_par "${output_directory}/par/frames.par")
set(generated_script "${output_directory}/frames.bat")
set(generated_compose_script "${output_directory}/compose.bat")
set(gold_compose_script "${GOLD_COMPOSE_SCRIPT}")
set(valid_config "${input_directory}/${TEST_NAME}.json")

file(REMOVE_RECURSE "${input_directory}" "${output_directory}")
file(MAKE_DIRECTORY "${input_directory}")
foreach(input_file
        "${TEST_NAME}.json"
        "source.par")
    file(COPY_FILE
        "${TEST_SOURCE_DIR}/${input_file}"
        "${input_directory}/${input_file}")
endforeach()
file(COPY_FILE "${CORE_CATALOG}" "${input_directory}/core-catalog.json")
file(COPY_FILE "${COLORING_CATALOG}" "${input_directory}/coloring-catalog.json")
if(DEFINED INDEXED_MAP AND NOT INDEXED_MAP STREQUAL "")
    set(indexed_map "${input_directory}/${INDEXED_MAP}")
    file(WRITE "${indexed_map}" "")
    foreach(index RANGE 0 255)
        math(EXPR green "255 - ${index}")
        math(EXPR blue "${index} % 64")
        file(APPEND "${indexed_map}" "${index} ${green} ${blue}\n")
    endforeach()
endif()

execute_process(
    COMMAND "${PARANIMATOR}" "${valid_config}"
    RESULT_VARIABLE generate_result
    OUTPUT_VARIABLE generate_output
    ERROR_VARIABLE generate_error
)
if(NOT generate_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to generate PAR file:\n"
        "${generate_output}\n"
        "${generate_error}"
    )
endif()

if(NOT EXISTS "${generated_par}")
    message(FATAL_ERROR "Generated PAR file was not created: ${generated_par}")
endif()

if(NOT EXISTS "${generated_script}")
    message(FATAL_ERROR
        "Generated script file was not created: ${generated_script}")
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}" -E compare_files
        "${GOLD_SCRIPT}"
        "${generated_script}"
    RESULT_VARIABLE script_compare_result
)
if(NOT script_compare_result EQUAL 0)
    message(FATAL_ERROR
        "Generated script file does not match gold script file:\n"
        "  gold: ${GOLD_SCRIPT}\n"
        "  generated: ${generated_script}"
    )
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}" -E compare_files
        "${GOLD_PAR}"
        "${generated_par}"
    RESULT_VARIABLE compare_result
)
if(NOT compare_result EQUAL 0)
    message(FATAL_ERROR
        "Generated PAR file does not match gold PAR file:\n"
        "  gold: ${GOLD_PAR}\n"
        "  generated: ${generated_par}"
    )
endif()

if(DEFINED OUTPUT_MAP AND NOT OUTPUT_MAP STREQUAL "")
    set(generated_map "${output_directory}/map/${OUTPUT_MAP}")
    if(NOT EXISTS "${generated_map}")
        message(FATAL_ERROR "Generated map file was not created: ${generated_map}")
    endif()
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E compare_files
            "${GOLD_MAP}"
            "${generated_map}"
        RESULT_VARIABLE map_compare_result
    )
    if(NOT map_compare_result EQUAL 0)
        message(FATAL_ERROR
            "Generated map file does not match gold map file:\n"
            "  gold: ${GOLD_MAP}\n"
            "  generated: ${generated_map}"
        )
    endif()
endif()

if(EXISTS "${gold_compose_script}")
    if(NOT EXISTS "${generated_compose_script}")
        message(FATAL_ERROR
            "Generated compose script file was not created: ${generated_compose_script}")
    endif()
    execute_process(
        COMMAND
            "${CMAKE_COMMAND}" -E compare_files
            "${gold_compose_script}"
            "${generated_compose_script}"
        RESULT_VARIABLE compose_compare_result
    )
    if(NOT compose_compare_result EQUAL 0)
        message(FATAL_ERROR
            "Generated compose script file does not match gold script file:\n"
            "  gold: ${gold_compose_script}\n"
            "  generated: ${generated_compose_script}"
        )
    endif()
endif()
