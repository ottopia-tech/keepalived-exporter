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

    bool AddIpAddress(const std::string& ipAddress, const std::string& netmask) const;

    bool DeleteIpAddress(const std::string& ipAddress) const;

private:
    bool set_ip_address(uint32_t ip_address) const;
    bool set_netmask(uint32_t netmask) const;
    uint32_t network_uint32_ipv4_from_string(const std::string& str_ip_address) const;

    std::shared_ptr<ILogger> m_logger;
    std::string m_name;
    int m_fd;
};
