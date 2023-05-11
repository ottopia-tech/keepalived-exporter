#pragma once

#include <string>
#include <memory>

struct ifaddrs;

class ILogger;

class InterfaceAddresses
{
public:
    InterfaceAddresses(const std::string& name, std::shared_ptr<ILogger> logger);
    ~InterfaceAddresses();
    bool ipAddressExists(const std::string& ipAddress);

private:
    std::shared_ptr<ILogger> m_logger;
    std::string m_name;
    ifaddrs* m_ifaddr;
};
