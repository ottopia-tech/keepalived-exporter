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

#define LOG(logger, message) logger->log(message)
#define LOG_ERROR(logger, message) (logger)->log(ILogger::LogLevel::Error, __PRETTY_FUNCTION__, __LINE__, (message))
#define LOG_INFO(logger, message) (logger)->log(ILogger::LogLevel::Info, __PRETTY_FUNCTION__, __LINE__, (message))
#define LOG_DEBUG(logger, message) (logger)->log(ILogger::LogLevel::Debug, __PRETTY_FUNCTION__, __LINE__, (message))