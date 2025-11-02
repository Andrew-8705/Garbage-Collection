cmake_minimum_required(VERSION 3.10)

project(gc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gc
    src/main.cpp
)

target_include_directories(gc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)