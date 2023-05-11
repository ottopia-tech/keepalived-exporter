#include <cstring>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ilogger.h"
#include "interface_addresses.h"

InterfaceAddresses::InterfaceAddresses(const std::string& name, std::shared_ptr<ILogger> logger) :
        m_logger(logger),
        m_name(name),
        m_ifaddr(nullptr)
{
    if (getifaddrs(&m_ifaddr) == -1)
    {
        perror("getifaddrs");
        freeifaddrs(m_ifaddr);
        m_ifaddr = nullptr;
        LOG_ERROR(m_logger, "getifaddrs failed");
    }
}

InterfaceAddresses::~InterfaceAddresses()
{
    freeifaddrs(m_ifaddr);
    m_ifaddr = nullptr;
}

bool InterfaceAddresses::ipAddressExists(const std::string& ipAddress)
{
    ifaddrs* ifa;
    if (m_ifaddr == nullptr)
    {
        LOG_ERROR(m_logger, "m_ifaddr is nullptr");
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
        if (strcmp(inet_ntoa(addr->sin_addr), ipAddress.c_str()) == 0)
        {
            return true;
        }
    }

    return false;
}
