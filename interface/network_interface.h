#pragma once

#include <string>
#include <memory>

class ILogger;

// Class for managing network interfaces
class NetworkInterface
{
public:
    NetworkInterface(const std::string& name);

    ~NetworkInterface();

    bool addIpAddress(const char* ipAddress, int prefixLen, std::shared_ptr<ILogger> logger);

    bool deleteIpAddress(const char* ipAddress, std::shared_ptr<ILogger> logger);

private:
    std::string m_name;
    int m_fd;
};
