#include <fstream>
#include <chrono>
#include <iomanip>

#include "file_logger.h"

FileLogger::FileLogger(const std::string& filename) :
        m_filename(filename)
{
}

void FileLogger::log(LogLevel level, const std::string& function, int line, const std::string& message)
{
    std::ofstream file(m_filename, std::ios_base::app);
    if (file)
    {
        // Get the current time.
        auto now = std::chrono::system_clock::now();

        // Convert the current time to a `time_t` value.
        time_t t = std::chrono::system_clock::to_time_t(now);
        std::string time_now = ctime(&t);
        time_now.resize(time_now.size() - 1);

        // Write the log message to the file.
        file << "[" << time_now << "] [" << ILogger::to_string(level) << "] [" << function << ":" << line << "] " << message << std::endl;
        file.close();
    }
}
