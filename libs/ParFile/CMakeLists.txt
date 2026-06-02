# SPDX-License-Identifier: GPL-3.0-only
#
find_package(boost_algorithm REQUIRED CONFIG)
find_package(boost_format REQUIRED CONFIG)
find_package(boost_json REQUIRED CONFIG)

add_library(ParFile
    include/ParFile/Config.h
    include/ParFile/Interpolant.h
    include/ParFile/Interpolator.h
    include/ParFile/Json.h
    include/ParFile/OutputLayout.h
    include/ParFile/ParFile.h
    include/ParFile/Script.h
    Config.cpp
    Interpolant.cpp
    Interpolator.cpp
    Json.cpp
    OutputLayout.cpp
    ParFile.cpp
    Script.cpp
)
target_include_directories(ParFile PUBLIC include)
target_link_libraries(ParFile PUBLIC Boost::algorithm Boost::format Boost::json)
target_folder(ParFile "Libraries")
