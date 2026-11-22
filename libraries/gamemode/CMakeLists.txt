cmake_minimum_required(VERSION 3.15)
project(gamemode
    VERSION 1.6.1)

add_library(gamemode INTERFACE)
target_include_directories(gamemode INTERFACE include)
target_link_libraries(gamemode INTERFACE ${CMAKE_DL_LIBS})
