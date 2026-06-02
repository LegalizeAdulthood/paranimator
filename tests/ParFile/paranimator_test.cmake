# SPDX-License-Identifier: GPL-3.0-only
#
foreach(name PARANIMATOR VALID_CONFIG INVALID_CONFIG GOLD_PAR)
    if(NOT DEFINED ${name})
        message(FATAL_ERROR "Missing required variable: ${name}")
    endif()
endforeach()

set(output_directory "e2e-output")
set(generated_par "${output_directory}/par/frames.par")
set(normalized_gold_par "gold-center-mag-normalized.par")

execute_process(
    COMMAND "${PARANIMATOR}" "${INVALID_CONFIG}"
    RESULT_VARIABLE invalid_result
    OUTPUT_VARIABLE invalid_output
    ERROR_VARIABLE invalid_error
)
if(invalid_result EQUAL 0)
    message(FATAL_ERROR "Schema-invalid config was accepted")
endif()

file(REMOVE_RECURSE "${output_directory}")
execute_process(
    COMMAND "${PARANIMATOR}" "${VALID_CONFIG}"
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

file(READ "${GOLD_PAR}" gold_text)
string(REGEX REPLACE "^;[^\n]*\n;[^\n]*\n" "" gold_text "${gold_text}")
file(WRITE "${normalized_gold_par}" "${gold_text}")
file(APPEND "${normalized_gold_par}" "\n")

execute_process(
    COMMAND
        "${CMAKE_COMMAND}" -E compare_files
        "${normalized_gold_par}"
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
