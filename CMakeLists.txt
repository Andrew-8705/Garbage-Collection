cmake_minimum_required(VERSION 3.10)

project(gc_framework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Настройки ---
option(STL_BENCH "Build the standard library (STL) workload." ON)
option(GRAPH_BENCH "Build the random graph workload." ON)
option(ENABLE_BDWGC "Enable Boehm-GC integration." ON)
option(ENABLE_TRACY "Enable Tracy profiler integration." OFF)
option(ENABLE_LOGGING "Enable detailed CSV and TXT logging." ON)

find_package(Threads REQUIRED)

# ==============================================================================
# 1. Библиотека ядра (GC Core)
# ==============================================================================

set(CORE_SOURCES
    src/core/MemoryManager.cpp
    src/impl/ReferenceCountingGC.cpp
    # src/utils/logger.cpp 
)

if(ENABLE_BDWGC)
    list(APPEND CORE_SOURCES src/impl/BoehmGCAdapter.cpp)
endif()

if(ENABLE_TRACY)
    message(STATUS "Tracy Profiler is ENABLED.")
    set(TRACY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/tracy/public")
    
    list(APPEND CORE_SOURCES "${TRACY_DIR}/TracyClient.cpp")
endif()

add_library(gc_core STATIC ${CORE_SOURCES})

target_include_directories(gc_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(gc_core PUBLIC Threads::Threads)

# --- Настройка логирования ---
if(ENABLE_LOGGING)
    message(STATUS "Logging is ENABLED.")
    target_compile_definitions(gc_core PUBLIC GC_ENABLE_LOGGING)
else()
    message(STATUS "Logging is DISABLED.")
endif()

# --- Настройка Tracy ---
if(ENABLE_TRACY)
    target_include_directories(gc_core PUBLIC "${TRACY_DIR}")
    target_compile_definitions(gc_core PUBLIC TRACY_ENABLE) 
    
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_libraries(gc_core PUBLIC dl)
    endif()
endif()

# --- Настройка Boehm GC ---
if(ENABLE_BDWGC)
    message(STATUS "Boehm-GC is ENABLED.")
    
    set(BDWGC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/bdwgc")
    set(BDWGC_LIB_DIR "${BDWGC_ROOT}/build") 
    
    find_library(BDWGC_LIB 
        NAMES gc libgc 
        PATHS ${BDWGC_LIB_DIR} 
        NO_DEFAULT_PATH
    )
    
    if(NOT BDWGC_LIB)
        message(FATAL_ERROR "Could not find libgc.a in ${BDWGC_LIB_DIR}")
    endif()

    target_include_directories(gc_core PUBLIC "${BDWGC_ROOT}/include")
    target_link_libraries(gc_core PUBLIC ${BDWGC_LIB} pthread)
    target_compile_definitions(gc_core PUBLIC HAS_BDWGC)
endif()

# ==============================================================================
# 2. Сборка бенчмарков
# ==============================================================================

if(STL_BENCH)
    message(STATUS "Configuring STL Benchmark")
    add_executable(stl_bench benchmarks/stl/stl_bench.cpp)
    target_link_libraries(stl_bench PRIVATE gc_core)
endif()

if (GRAPH_BENCH)
    message(STATUS "Configuring Graph Benchmark")
    add_executable(graph_bench benchmarks/graph/graph_bench.cpp)
    target_link_libraries(graph_bench PRIVATE gc_core)
endif()