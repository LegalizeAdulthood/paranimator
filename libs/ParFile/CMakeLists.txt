find_package(boost_algorithm REQUIRED CONFIG)
find_package(boost_format REQUIRED CONFIG)
find_package(boost_json REQUIRED CONFIG)
find_package(Tweeny REQUIRED CONFIG)

add_library(ParFile
    include/ParFile/Config.h
    include/ParFile/Interpolator.h
    include/ParFile/Json.h
    include/ParFile/ParFile.h
    Config.cpp
    Json.cpp
    ParFile.cpp
    Interpolator.cpp
)
target_include_directories(ParFile PUBLIC include)
target_link_libraries(ParFile PUBLIC tweeny Boost::algorithm Boost::format Boost::json)
target_folder(ParFile "Libraries")
