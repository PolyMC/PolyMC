project(md4c LANGUAGES C)

add_library(md4c STATIC md4c.c md4c.h)
target_include_directories(md4c PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
