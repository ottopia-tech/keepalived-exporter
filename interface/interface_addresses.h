#pragma once

#include <string>

struct ifaddrs;

class ILogger;

class InterfaceAddresses
{
public:
    InterfaceAddresses(const std::string& name);
    ~InterfaceAddresses();
    bool ipAddressExists(const std::string& ipAddress);

private:
    std::string m_name;
    ifaddrs* m_ifaddr;
};
