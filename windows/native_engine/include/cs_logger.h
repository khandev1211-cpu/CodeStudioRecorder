#pragma once
#include <string>
#include <mutex>
#include <fstream>
#include <iostream>

namespace cs {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static Logger& instance();

    void log(LogLevel level, const std::string& message);
    void setLogFile(const std::string& path);

private:
    Logger() = default;
    std::mutex mutex_;
    std::ofstream file_stream_;

    std::string levelToString(LogLevel level);
};

#define CS_LOG_DEBUG(msg) ::cs::Logger::instance().log(::cs::LogLevel::Debug, msg)
#define CS_LOG_INFO(msg) ::cs::Logger::instance().log(::cs::LogLevel::Info, msg)
#define CS_LOG_WARN(msg) ::cs::Logger::instance().log(::cs::LogLevel::Warning, msg)
#define CS_LOG_ERR(msg) ::cs::Logger::instance().log(::cs::LogLevel::Error, msg)

} // namespace cs
