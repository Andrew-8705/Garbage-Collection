cmake_minimum_required(VERSION 3.10)

project(gc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(GC_DEBUG_ALLOC "Enable global operator new/delete override for memory tracking." OFF)
option(ROCKSDB_BENCH "Build the RocksDB workload executable." OFF)
option(STL_BENCH "Build the standard library (STL) workload executable." OFF)
option(BDWGC_BENCH "Build the Boehm-Demers-Weiser GC workload executable." OFF)

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
# --- УСЛОВНОЕ ДОБАВЛЕНИЕ БЕНЧМАРКА ROCKSDB ---
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
# --- УСЛОВНОЕ ДОБАВЛЕНИЕ БЕНЧМАРКА STL ---
# -------------------------------------------------------------------

if(STL_BENCH)
    message(STATUS "STL benchmark is ENABLED.")
    
    set(STL_BENCH_SOURCES 
        src/benchmarking/stl_bench.cpp       # Исходный файл с нагрузкой STL
        src/core/memory_override.cpp         # Ваш аллокатор/GC
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

# -------------------------------------------------------------------
# --- ИНТЕГРАЦИЯ BOEHM-GC (BDWGC) ---
# -------------------------------------------------------------------
if(BDWGC_BENCH)
    message(STATUS "Boehm-Demers-Weiser GC is ENABLED.")

    set(BDWGC_DIR "/Users/andrew_baldin/Desktop/Diploma_work/GC/Garbage-Collection/third_party/rocksdb/build") 
    # Путь к собранной библиотеке libgc.a
    set(BDWGC_LIBRARY "${BDWGC_DIR}/build/libgc.a") 
    
    set(BDWGC_BENCH_SOURCES 
        src/core/memory_override.cpp
    )

    add_executable(bdwgc_bench ${BDWGC_BENCH_SOURCES}) 

    if(GC_DEBUG_ALLOC)
        target_compile_definitions(bdwgc_bench PRIVATE GC_DEBUG_ALLOC)
    endif()

    # Включение заголовочных файлов:
    # 1. Ваши собственные (include)
    # 2. Заголовочные файлы BDWGC (BDWGC_DIR/include)
    target_include_directories(bdwgc_bench PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        "${BDWGC_DIR}/include"
    )
    
    # Связывание с библиотекой:
    # 1. Сама библиотека BDWGC
    # 2. -lpthread (обычно требуется даже для однопоточной консервативной работы, 
    #    поскольку GC может использовать атомарные операции или потоки для работы с корнями)
    target_link_libraries(bdwgc_bench
        ${BDWGC_DIR}/
        -lpthread 
    )
    
else()
    message(STATUS "Boehm-Demers-Weiser GC benchmark is DISABLED. Set -D BDWGC_BENCH=ON to enable.")
endif()