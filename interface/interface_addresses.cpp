#include <cstring>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "interface_addresses.h"

InterfaceAddresses::InterfaceAddresses(const std::string& name) :
        m_name(name),
        m_ifaddr(nullptr)
{
    if (getifaddrs(&m_ifaddr) == -1)
    {
        perror("getifaddrs");
        freeifaddrs(m_ifaddr);
        m_ifaddr = nullptr;
    }
}

InterfaceAddresses::~InterfaceAddresses()
{
    freeifaddrs(m_ifaddr);
    m_ifaddr = nullptr;
}

bool InterfaceAddresses::ipAddressExists(const char* ipAddress)
{
    ifaddrs* ifa;
    if (m_ifaddr == nullptr)
    {
        return false;
    }

    for (ifa = m_ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL || ifa->ifa_name == NULL || strcmp(ifa->ifa_name, m_name.c_str()) != 0 ||
            ifa->ifa_addr->sa_family != AF_INET)
        {
            continue;
        }

        sockaddr_in* addr = (sockaddr_in*) ifa->ifa_addr;
        std::string address = inet_ntoa(addr->sin_addr);
        if (strcmp(inet_ntoa(addr->sin_addr), ipAddress) == 0)
        {
            return true;
        }
    }

    return false;
}
