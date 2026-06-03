# SPDX-License-Identifier: GPL-3.0-only
#
foreach(name PAR_CONFIG PARANIMATOR TEST_MODE TEST_ENTRY TEST_FRAMES TEST_SOURCE_DIR CORE_CATALOG GOLD_JSON)
    if(NOT DEFINED ${name})
        message(FATAL_ERROR "Missing required variable: ${name}")
    endif()
endforeach()

set(input_directory "input")
set(generated_config "generated.json")

file(REMOVE_RECURSE "${input_directory}" "output" "${generated_config}")
file(MAKE_DIRECTORY "${input_directory}")
foreach(input_file
        "from.par"
        "to.par")
    file(COPY_FILE
        "${TEST_SOURCE_DIR}/${input_file}"
        "${input_directory}/${input_file}")
endforeach()
file(COPY_FILE "${CORE_CATALOG}" "core-catalog.json")

execute_process(
    COMMAND
        "${PAR_CONFIG}"
        "@input/from.par/${TEST_ENTRY}"
        "@input/to.par/${TEST_ENTRY}"
        "${TEST_FRAMES}"
        "${TEST_MODE}"
    RESULT_VARIABLE generate_result
    OUTPUT_FILE "${generated_config}"
    ERROR_VARIABLE generate_error
)
if(NOT generate_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to generate starter config:\n"
        "${generate_error}"
    )
endif()

execute_process(
    COMMAND
        "${CMAKE_COMMAND}" -E compare_files
        "${GOLD_JSON}"
        "${generated_config}"
    RESULT_VARIABLE compare_result
)
if(NOT compare_result EQUAL 0)
    message(FATAL_ERROR
        "Generated starter config does not match gold JSON:\n"
        "  gold: ${GOLD_JSON}\n"
        "  generated: ${generated_config}"
    )
endif()

execute_process(
    COMMAND "${PARANIMATOR}" "${generated_config}"
    RESULT_VARIABLE validate_result
    OUTPUT_VARIABLE validate_output
    ERROR_VARIABLE validate_error
)
if(NOT validate_result EQUAL 0)
    message(FATAL_ERROR
        "Generated starter config was not accepted by paranimator:\n"
        "${validate_output}\n"
        "${validate_error}"
    )
endif()
