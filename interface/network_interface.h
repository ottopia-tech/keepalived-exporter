#pragma once

#include <string>
#include <memory>

class ILogger;

// Class for managing network interfaces
class NetworkInterface
{
public:
    NetworkInterface(const std::string& name, std::shared_ptr<ILogger> logger);

    ~NetworkInterface();

    bool addIpAddress(const std::string& ipAddress);

    bool deleteIpAddress(const std::string& ipAddress);

private:
    std::shared_ptr<ILogger> m_logger;
    std::string m_name;
    int m_fd;
};
