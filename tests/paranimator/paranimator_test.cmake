# SPDX-License-Identifier: GPL-3.0-only
#
foreach(name PARANIMATOR TEST_NAME TEST_FRAMES TEST_SOURCE_DIR CORE_CATALOG GOLD_PAR)
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
        "Generated batch file was not created: ${generated_script}")
endif()

file(READ "${generated_script}" generated_script_text)
string(FIND "${generated_script_text}" "librarydirs=${output_directory}" found)
if(found EQUAL -1)
    message(FATAL_ERROR
        "Generated batch file is missing expected text: librarydirs=${output_directory}")
endif()
foreach(frame RANGE 1 ${TEST_FRAMES})
    string(REGEX REPLACE "^0*([0-9]+)$" "\\1" frame_number "${frame}")
    string(LENGTH "${frame_number}" frame_number_length)
    string(SUBSTRING "0000${frame_number}" "${frame_number_length}" 4 frame_text)
    set(expected "@frames.par/frame-${frame_text}")
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
