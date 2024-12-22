find_package(boost_format REQUIRED CONFIG)
find_package(boost_json REQUIRED CONFIG)
find_package(Tweeny REQUIRED CONFIG)

add_executable(paranimator
    main.cpp
)
target_folder(paranimator "Tools")
target_link_libraries(paranimator PUBLIC ParFile tweeny Boost::format Boost::json)
