#include <fstream>
#include <ctime>

#include "file_logger.h"

FileLogger::FileLogger(const std::string& filename) :
        m_filename(filename)
{
}

void FileLogger::log(const std::string& message)
{
    std::ofstream file(m_filename, std::ios_base::app);
    if (file)
    {
        time_t now = time(0);
        char* dt = ctime(&now);
        file << dt << ": " << message << std::endl;
        file.close();
    }
}
