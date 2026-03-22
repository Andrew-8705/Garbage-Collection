#pragma once

#include <cstdio>
#include <mutex>
#include <string>
#include <chrono>
#include <iostream>
#include <cstdarg>

class LogChannel {
public:
    LogChannel(const char* filename, const char* header = nullptr) {
        file = std::fopen(filename, "w");
        if (file) {
            if (header) {
                std::fprintf(file, "%s\n", header);
                std::fflush(file);
            }
        } else {
            std::cerr << "[Logger] CRITICAL ERROR: Cant open " << filename << "\n";
        }
    }

    ~LogChannel() {
        if (file) std::fclose(file);
    }

    void write(const char* format, ...) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!file) return;

        auto now = std::chrono::steady_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

        std::fprintf(file, "%lld,", us);

        va_list args;
        va_start(args, format);
        std::vfprintf(file, format, args);
        va_end(args);

        std::fprintf(file, "\n");
    }
    

    void writeText(const char* format, ...) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!file) return;

        auto now = std::chrono::steady_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();

        std::fprintf(file, "[%lld] ", us);

        va_list args;
        va_start(args, format);
        std::vfprintf(file, format, args);
        va_end(args);

        std::fprintf(file, "\n");
        std::fflush(file);
    }

private:
    std::FILE* file = nullptr;
    std::mutex mtx;
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // --- Канал 1: Метрики памяти (CSV) ---
    void logAlloc(size_t size, void* ptr) {
        // Формат: TIMESTAMP,ALLOC,size,ptr
        memoryChannel.write("ALLOC,%zu,%p", size, ptr);
    }

    void logFree(void* ptr) {
        memoryChannel.write("FREE,0,%p", ptr);
    }

    void logHeapState(size_t live_bytes) {
        // Формат: TIMESTAMP, HEAP, live_bytes, 0x0
        memoryChannel.write("HEAP,%zu,0x0", live_bytes);
    }

    // --- Канал 2: Граф (CSV) ---
    void logGraphEvent(const char* event, int id1, int id2 = -1) {
        graphChannel.write("%s,%d,%d", event, id1, id2);
    }

    // --- Канал 3: Текстовый лог ---
    void log(const char* message) {
        textChannel.writeText("%s", message);
    }

    void logFormatted(const char* format, ...) {
        char buffer[1024];
        va_list args;
        va_start(args, format);
        std::vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        
        textChannel.writeText("%s", buffer);
    }

    Logger(const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;

private:
    Logger() 
        : memoryChannel("./results/data/memory_log.csv", "timestamp_us,event,size,ptr"),
          graphChannel("./results/data/graph_log.csv", "timestamp_us,event,id1,id2"),
          textChannel("./results/data/execution_log.txt", nullptr) 
    {}

    ~Logger() = default;

    // Каналы логирования
    LogChannel memoryChannel;
    LogChannel graphChannel;
    LogChannel textChannel;
};