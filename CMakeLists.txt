cmake_minimum_required(VERSION 3.10)

project(gc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GC_DEBUG_ALLOC "Enable global operator new/delete override for memory tracking." OFF)
option(ROCKSDB_BENCH "Build the RocksDB workload executable." OFF)
option(STL_BENCH "Build the standard library (STL) workload executable." OFF)

# --- 1. Основной исполняемый файл GC ---
set(CORE_SOURCES 
    src/core/main.cpp
    src/core/memory_override.cpp 
)

add_executable(gc ${CORE_SOURCES})

if(GC_DEBUG_ALLOC)
    target_compile_definitions(gc PRIVATE GC_DEBUG_ALLOC)
endif()

target_include_directories(gc PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# -------------------------------------------------------------------
# --- 2. УСЛОВНОЕ ДОБАВЛЕНИЕ БЕНЧМАРКА ROCKSDB ---
# -------------------------------------------------------------------

if(ROCKSDB_BENCH)
    message(STATUS "RocksDB benchmark is ENABLED.")
    
    set(ROCKSDB_DIR "/Users/andrew_baldin/Desktop/Diploma_work/GC/Garbage-Collection/third_party/rocksdb/build")

    set(BENCH_SOURCES 
        src/core/rocksdb_bench.cpp
        src/core/memory_override.cpp 
    )

    add_executable(rocksdb_bench ${BENCH_SOURCES})

    if(GC_DEBUG_ALLOC)
        target_compile_definitions(rocksdb_bench PRIVATE GC_DEBUG_ALLOC)
    endif()

    target_include_directories(rocksdb_bench PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${ROCKSDB_DIR}/../include
    )

    target_link_libraries(rocksdb_bench 
        # Основная библиотека RocksDB
        ${ROCKSDB_DIR}/librocksdb.a 
        
        # Зависимости
        -L/opt/homebrew/opt/snappy/lib -lsnappy
        -L/opt/homebrew/opt/lz4/lib -llz4
        -L/opt/homebrew/opt/zstd/lib -lzstd
        -L/opt/homebrew/opt/gflags/lib -lgflags
        
        # Системные и стандартные зависимости
        -lpthread 
        -lz  
    )

else()
    message(STATUS "RocksDB benchmark is DISABLED. Set -D ROCKSDB_BENCH=ON to enable.")
endif()

# -------------------------------------------------------------------
# --- 3. УСЛОВНОЕ ДОБАВЛЕНИЕ БЕНЧМАРКА STL ---
# -------------------------------------------------------------------

if(STL_BENCH)
    message(STATUS "STL benchmark is ENABLED.")
    
    set(STL_BENCH_SOURCES 
        src/benchmarking/stl_bench.cpp       # Исходный файл с нагрузкой STL
        src/core/memory_override.cpp # Ваш аллокатор/GC
    )

    add_executable(stl_bench ${STL_BENCH_SOURCES}) # <--- НОВЫЙ ТАРГЕТ

    if(GC_DEBUG_ALLOC)
        target_compile_definitions(stl_bench PRIVATE GC_DEBUG_ALLOC)
    endif()

    target_include_directories(stl_bench PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    
    target_link_libraries(stl_bench) 
    
else()
    message(STATUS "STL benchmark is DISABLED. Set -D STL_BENCH=ON to enable.")
endif()