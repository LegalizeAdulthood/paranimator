# SPDX-License-Identifier: GPL-3.0-only
#
foreach(name PARANIMATOR TEST_NAME TEST_SOURCE_DIR GOLD_PAR)
    if(NOT DEFINED ${name})
        message(FATAL_ERROR "Missing required variable: ${name}")
    endif()
endforeach()

set(input_directory "input")
set(output_directory "output")
set(generated_par "${output_directory}/par/frames.par")
set(valid_config "${input_directory}/${TEST_NAME}.json")

file(REMOVE_RECURSE "${input_directory}" "${output_directory}")
file(MAKE_DIRECTORY "${input_directory}")
foreach(input_file
        "${TEST_NAME}.json"
        "source.par"
        "viewport-catalog.json")
    file(COPY_FILE
        "${TEST_SOURCE_DIR}/${input_file}"
        "${input_directory}/${input_file}")
endforeach()

execute_process(
    COMMAND "${PARANIMATOR}" "${valid_config}"
    RESULT_VARIABLE generate_result
    OUTPUT_VARIABLE generate_output
    ERROR_VARIABLE generate_error
)
if(NOT generate_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to generate center-mag PAR file:\n"
        "${generate_output}\n"
        "${generate_error}"
    )
endif()

if(NOT EXISTS "${generated_par}")
    message(FATAL_ERROR "Generated PAR file was not created: ${generated_par}")
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
