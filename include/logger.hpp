#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <mutex>

class Logger {
public:
    static Logger& getInstance(const char* filepath = "./results/data/log.txt") {
        static Logger instance(filepath);
        return instance;
    }

    void log(const char* message) {
        std::lock_guard<std::mutex> lock(mtx);
        if (file) {
            std::fprintf(file, "%s\n", message);
        }
    }

    void log(long long value) {
        std::lock_guard<std::mutex> lock(mtx);
        if (file) {
            std::fprintf(file, "%lld\n", value);
        }
    }

    Logger(const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;

private:
    explicit Logger(const char* filepath) {
        file = std::fopen(filepath, "a");
        if (!file) {
            std::fprintf(stderr, "CRITICAL ERROR: Cant open log file: %s\n", filepath);
        }
    }

    ~Logger() { 
        if (file) {
            std::fclose(file); 
        }
    }

    std::FILE* file = nullptr;
    std::mutex mtx;
}; 