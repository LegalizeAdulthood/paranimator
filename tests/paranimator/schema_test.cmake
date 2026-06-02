# SPDX-License-Identifier: GPL-3.0-only
#
foreach(name PARANIMATOR TEST_NAME TEST_SOURCE_DIR CORE_CATALOG)
    if(NOT DEFINED ${name})
        message(FATAL_ERROR "Missing required variable: ${name}")
    endif()
endforeach()

set(input_directory "input")
set(config "${input_directory}/${TEST_NAME}.json")

file(REMOVE_RECURSE "${input_directory}" "output")
file(MAKE_DIRECTORY "${input_directory}")
foreach(input_file
        "${TEST_NAME}.json"
        "source.par")
    file(COPY_FILE
        "${TEST_SOURCE_DIR}/${input_file}"
        "${input_directory}/${input_file}")
endforeach()
file(COPY_FILE "${CORE_CATALOG}" "${input_directory}/core-catalog.json")

execute_process(
    COMMAND "${PARANIMATOR}" "${config}"
    RESULT_VARIABLE invalid_result
    OUTPUT_VARIABLE invalid_output
    ERROR_VARIABLE invalid_error
)
if(invalid_result EQUAL 0)
    message(FATAL_ERROR "Schema-invalid config was accepted")
endif()
