find_package(boost_algorithm REQUIRED CONFIG)

add_library(ParFile ParFile.cpp include/ParFile/ParFile.h)
target_include_directories(ParFile PUBLIC include)
target_link_libraries(ParFile PUBLIC Boost::algorithm)
target_folder(ParFile "Libraries")
