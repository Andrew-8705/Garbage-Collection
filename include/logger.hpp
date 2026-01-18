#pragma once

#include <cstdio>
#include <mutex>
#include <string>
#include <chrono>
#include <iostream>
#include <cstdarg>

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // --- Канал 1: Метрики памяти (CSV) ---
    void logAlloc(size_t size, void* ptr) {
        std::lock_guard<std::mutex> lock(csv_mtx);
        if (csv_file) {
            auto us = getCurrentTimeUs();
            std::fprintf(csv_file, "%lld,ALLOC,%zu,%p\n", us, size, ptr);
        }
    }

    void logFree(void* ptr) {
        std::lock_guard<std::mutex> lock(csv_mtx);
        if (csv_file) {
            auto us = getCurrentTimeUs();
            std::fprintf(csv_file, "%lld,FREE,0,%p\n", us, ptr);
        }
    }

    // --- Канал 2: Информационный лог (TXT) ---
    void log(const char* message) {
        std::lock_guard<std::mutex> lock(text_mtx);
        

        if (text_file) {
            auto us = getCurrentTimeUs();
            std::fprintf(text_file, "[%lld] %s\n", us, message);
            std::fflush(text_file);
        }
    }

    void log(const char* prefix, int value) {
        std::lock_guard<std::mutex> lock(text_mtx);
        if (text_file) {
            auto us = getCurrentTimeUs();
            std::fprintf(text_file, "[%lld] %s: %d\n", us, prefix, value);
            std::fflush(text_file);
        }
    }

    void logFormatted(const char* format, ...) {
    std::lock_guard<std::mutex> lock(text_mtx);
    if (text_file) {
        auto us = getCurrentTimeUs();
        std::fprintf(text_file, "[%lld] ", us);
        
        va_list args;
        va_start(args, format);
        std::vfprintf(text_file, format, args);
        va_end(args);
        
        std::fprintf(text_file, "\n");
        std::fflush(text_file);
    }
}

    Logger(const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;

private:
    Logger() {
        // 1. Файл для графиков
        csv_file = std::fopen("./results/data/memory_log.csv", "w");
        if (csv_file) {
            std::fprintf(csv_file, "timestamp_us,event,size,ptr\n");
        } else {
            std::cerr << "CRITICAL ERROR: Cant open ./results/data/memory_log.csv\n";
        }

        // 2. Файл для информации
        text_file = std::fopen("./results/data/execution_log.txt", "w");
        if (!text_file) {
            std::cerr << "CRITICAL ERROR: Cant open ./results/data/execution_log.txt\n";
        }
    }

    ~Logger() {
        if (csv_file) std::fclose(csv_file);
        if (text_file) std::fclose(text_file);
    }

    long long getCurrentTimeUs() {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
    }

    std::FILE* csv_file = nullptr;
    std::FILE* text_file = nullptr;
    
    std::mutex csv_mtx;
    std::mutex text_mtx;
};