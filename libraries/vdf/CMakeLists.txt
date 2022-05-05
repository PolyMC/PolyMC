project(vdf)

set(vdf_SOURCES
        include/vdf_parser.hpp
        )

add_library(vdf STATIC ${vdf_SOURCES})

target_include_directories(vdf PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
