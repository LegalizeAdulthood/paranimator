add_library(ParFile ParFile.cpp include/ParFile/ParFile.h)
target_include_directories(ParFile PUBLIC include)
set_target_properties(ParFile PROPERTIES FOLDER "Libraries")
