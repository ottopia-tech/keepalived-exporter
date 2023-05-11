#pragma once

#include <string>

#include "ilogger.h"

// Logger that logs messages to a file
class FileLogger : public ILogger
{
public:
    FileLogger(const std::string& filename);

    void log(LogLevel level, const std::string& function, int line, const std::string& message) override;

private:
    std::string m_filename;
};
