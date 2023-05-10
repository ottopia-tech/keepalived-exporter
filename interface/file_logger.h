#pragma once

#include <string>

#include "ilogger.h"

// Logger that logs messages to a file
class FileLogger : public ILogger {
public:
    FileLogger(const std::string& filename);

    void log(const std::string& message) override;

private:
    std::string m_filename;
};
