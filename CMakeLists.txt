cmake_minimum_required(VERSION 3.10)

project(gc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GC_DEBUG_ALLOC "Enable global operator new/delete override for memory tracking." OFF)

set(SOURCE_FILES 
    src/main.cpp
    src/memory_override.cpp 
)

add_executable(gc ${SOURCE_FILES})

if(GC_DEBUG_ALLOC)
    target_compile_definitions(gc PRIVATE GC_DEBUG_ALLOC) 
endif()

target_include_directories(gc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)