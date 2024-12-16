cmake_minimum_required(VERSION 3.23)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PARANIMATOR_PROJECT_NAME "ParAnimator" CACHE "STRING" "Default project name for IDEs")
project("${PARANIMATOR_PROJECT_NAME}" CXX)

include(CTest)

function(target_folder target folder)
    set_target_properties(${target} PROPERTIES FOLDER ${folder})
endfunction()

add_subdirectory(libs)
add_subdirectory(tools)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
