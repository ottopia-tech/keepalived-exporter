#pragma once

#include <string>
#include <unordered_map>

// Interface for logging messages
class ILogger
{
public:
    enum class LogLevel
    {
        Debug,
        Info,
        Error
    };

    virtual void log(LogLevel level, const std::string& function, int line, const std::string& message) = 0;
    virtual ~ILogger()
    {
    }

    static std::string to_string(LogLevel logLevel)
    {
        std::unordered_map<LogLevel, std::string> logLevelToString = {
                {LogLevel::Debug, "DEBUG"},
                {LogLevel::Info, "INFO"},
                {LogLevel::Error, "ERROR"}
        };
        return logLevelToString[logLevel];
    }
};

#define LOG(logger, level, message) (logger)->log((level), __PRETTY_FUNCTION__, __LINE__, (message))
#define LOG_ERROR(logger, message) LOG((logger), ILogger::LogLevel::Error, (message))
#define LOG_INFO(logger, message) LOG((logger), ILogger::LogLevel::Info, (message))
#define LOG_DEBUG(logger, message) LOG((logger), ILogger::LogLevel::Debug, (message))
