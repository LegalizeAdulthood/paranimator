# SPDX-License-Identifier: GPL-3.0-only
#
find_package(boost_algorithm REQUIRED CONFIG)
find_package(fmt REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED CONFIG)
find_package(nlohmann_json_schema_validator REQUIRED CONFIG)

add_library(ParFile
    include/ParFile/AnimationEnums.h
    include/ParFile/ColorMap.h
    include/ParFile/ColorSpec.h
    include/ParFile/ComposeScript.h
    include/ParFile/Config.h
    include/ParFile/Interpolant.h
    include/ParFile/Interpolator.h
    include/ParFile/JsonSchema.h
    include/ParFile/NumberTrack.h
    include/ParFile/OutputLayout.h
    include/ParFile/ParameterCatalog.h
    include/ParFile/ParFile.h
    include/ParFile/ResolvedAnimation.h
    include/ParFile/Script.h
    AnimationEnums.cpp
    ColorMap.cpp
    ColorSpec.cpp
    ComposeScript.cpp
    Config.cpp
    Interpolant.cpp
    Interpolator.cpp
    JsonSchema.cpp
    NumberTrack.cpp
    OutputLayout.cpp
    ParameterCatalog.cpp
    ParFile.cpp
    ResolvedAnimation.cpp
    Script.cpp
    StringUtil.cpp
    StringUtil.h
    "${PARFILE_SCRIPT_DIALECT_INCLUDE_DIR}/ParFile/ScriptDialect.h"
)
target_include_directories(ParFile
    PUBLIC include
    PRIVATE
        .
        "${PARFILE_SCRIPT_DIALECT_INCLUDE_DIR}"
)
target_link_libraries(
    ParFile
    PUBLIC
        Boost::algorithm
    PRIVATE
        fmt::fmt-header-only
        nlohmann_json::nlohmann_json
        nlohmann_json_schema_validator::validator
)
target_folder(ParFile "Libraries")
