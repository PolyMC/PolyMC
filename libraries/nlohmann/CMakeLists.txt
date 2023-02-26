cmake_minimum_required(VERSION 3.9.4)
project(nlohmann_json)

set(JSON_SOURCES
    include/nlohmann
)

add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE include/nlohmann)