# SPDX-License-Identifier: GPL-3.0-only
#
set(PARANIMATOR_CONFIG_SCHEMA_JSON "${CMAKE_SOURCE_DIR}/schemas/config.schema.json")
configure_file("ConfigSchema.h.in" "include/ConfigSchema.h" @ONLY)

add_executable(paranimator
    ConfigSchema.h.in
    main.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/include/ConfigSchema.h"
    "${CMAKE_SOURCE_DIR}/schemas/config.schema.json"
)
target_folder(paranimator "Tools")
target_include_directories(paranimator PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/include")
target_link_libraries(paranimator PUBLIC ParFile)
