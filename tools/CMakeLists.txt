# SPDX-License-Identifier: GPL-3.0-only
#
add_executable(paranimator
    main.cpp
)
target_folder(paranimator "Tools")
target_link_libraries(paranimator PUBLIC ParFile)
