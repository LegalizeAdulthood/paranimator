# SPDX-License-Identifier: GPL-3.0-only
#
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(par-config
    main.cpp
)
target_folder(par-config "Tools")
target_link_libraries(par-config PRIVATE ParFile nlohmann_json::nlohmann_json)
