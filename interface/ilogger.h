#pragma once

#include <string>

// Interface for logging messages
class ILogger {
public:
    virtual void log(const std::string& message) = 0;
    virtual ~ILogger() {}
};
