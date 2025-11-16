#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <mutex>

class Logger {
public:
    static Logger& getInstance(const std::string& filepath = "./results/log.txt") {
        static Logger instance(filepath);
        return instance;
    }

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);
        out << message << '\n';
    }

    Logger(const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;

private:
    explicit Logger(const std::string& filepath) : out(filepath, std::ios::app) {
        if (!out.is_open()) {
            throw std::runtime_error("Cant open the log file: " + filepath);
        }
    }

    ~Logger() { out.close(); }

    std::ofstream out;
    std::mutex mtx;
}; 