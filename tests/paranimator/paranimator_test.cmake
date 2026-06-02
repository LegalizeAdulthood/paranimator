# SPDX-License-Identifier: GPL-3.0-only
#
foreach(name PARANIMATOR TEST_NAME TEST_SOURCE_DIR CORE_CATALOG GOLD_PAR)
    if(NOT DEFINED ${name})
        message(FATAL_ERROR "Missing required variable: ${name}")
    endif()
endforeach()

set(input_directory "input")
set(output_directory "output")
set(generated_par "${output_directory}/par/frames.par")
set(generated_script "${output_directory}/frames.bat")
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
        "Generated batch file was not created: ${generated_script}")
endif()

file(READ "${generated_script}" generated_script_text)
foreach(expected
        "librarydirs=${output_directory}"
        "@frames.par/frame-0001"
        "@frames.par/frame-0002"
        "@frames.par/frame-0003")
    string(FIND "${generated_script_text}" "${expected}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR
            "Generated batch file is missing expected text: ${expected}")
    endif()
endforeach()

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
