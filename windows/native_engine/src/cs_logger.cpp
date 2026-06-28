#include "cs_logger.h"
#include <chrono>
#include <iomanip>
#include <sstream>

namespace cs {

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

void Logger::setLogFile(const std::string& path) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
    file_stream_.open(path, std::ios::app);
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::stringstream ss;
    ss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "."
       << std::setfill('0') << std::setw(3) << ms.count() << "] "
       << "[" << levelToString(level) << "] "
       << message << std::endl;

    std::string formatted = ss.str();

    // Log to console
    std::cout << formatted;

    // Log to file
    if (file_stream_.is_open()) {
        file_stream_ << formatted;
        file_stream_.flush();
    }
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
        default: return "UNKNOWN";
    }
}

} // namespace cs
