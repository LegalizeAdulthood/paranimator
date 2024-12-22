find_package(boost_algorithm REQUIRED CONFIG)

add_library(ParFile
    include/ParFile/Interpolator.h
    include/ParFile/ParFile.h
    ParFile.cpp
    Interpolator.cpp
)
target_include_directories(ParFile PUBLIC include)
target_link_libraries(ParFile PUBLIC Boost::algorithm)
target_folder(ParFile "Libraries")
