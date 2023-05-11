#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <sys/ioctl.h>

#include "network_interface.h"
#include "ilogger.h"
#include "interface_addresses.h"

NetworkInterface::NetworkInterface(const std::string& name, std::shared_ptr<ILogger> logger) :
        m_logger(logger),
        m_name(name)
{
    m_fd = socket(AF_INET, SOCK_DGRAM, 0);
}

NetworkInterface::~NetworkInterface()
{
    if (m_fd >= 0)
    {
        close(m_fd);
    }
}

bool NetworkInterface::AddIpAddress(const std::string& ipAddress, const std::string& netmask) const
{
    if (m_fd < 0)
    {
        perror("socket");
        LOG_ERROR(m_logger, "socket fd is negative: " + std::to_string(m_fd));
        return false;
    }

    uint32_t ip_address = network_uint32_ipv4_from_string(ipAddress);
    if (ip_address == 0)
    {
        return false;
    }
    uint32_t netmask_bin = network_uint32_ipv4_from_string(netmask);
    if (netmask_bin == 0)
    {
        return false;
    }

    if (!set_ip_address(ip_address))
    {
        return false;
    }
    LOG_INFO(m_logger, "successfully set ip: " + ipAddress + " on interface: " + m_name);

    if (!set_netmask(netmask_bin))
    {
        return false;
    }
    LOG_INFO(m_logger, "successfully set netmask: " + netmask + " on interface: " + m_name);

    return true;
}


bool NetworkInterface::DeleteIpAddress(const std::string& ipAddress) const
{
    if (m_fd < 0)
    {
        perror("socket");
        LOG_ERROR(m_logger, "socket fd is negative: " + std::to_string(m_fd));
        return false;
    }

    InterfaceAddresses interfaceAddresses(m_name, m_logger);
    // Check if the IP address exists on the interface
    if (!interfaceAddresses.ipAddressExists(ipAddress))
    {
        LOG_INFO(m_logger, "IP address " + ipAddress + " not found on interface " + m_name);
        return false;
    }

    // Set IP address to 0.0.0.0 to delete it
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    inet_aton("0.0.0.0", &sin.sin_addr);

    ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, m_name.c_str(), IFNAMSIZ - 1);
    memcpy(&ifr.ifr_addr, &sin, sizeof(sin));

    // Delete IP address from interface
    int ret_val = ioctl(m_fd, SIOCSIFADDR, &ifr);
    if (ret_val < 0)
    {
        LOG_ERROR(m_logger, "ioctl SIOCSIFADDR, ret_val: " + std::to_string(ret_val));
        return false;
    }

    LOG_INFO(m_logger, "successfully deleted ip: " + ipAddress + " on interface: " + m_name);
    return true;
}

bool NetworkInterface::set_ip_address(uint32_t ip_address) const
{
    if (m_fd < 0)
    {
        perror("socket");
        LOG_ERROR(m_logger, "socket fd is negative: " + std::to_string(m_fd));
        return false;
    }

    ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_ifru.ifru_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_ifrn.ifrn_name, m_name.c_str(), IFNAMSIZ - 1);

    constexpr size_t IP_START_INDEX = 2;

    memcpy(ifr.ifr_ifru.ifru_addr.sa_data + IP_START_INDEX, &ip_address, sizeof(ip_address));
    int success = ioctl(m_fd, SIOCSIFADDR, &ifr);
    if (success < 0)
    {
        LOG_ERROR(m_logger, "Configuring IPv4 address failed for interface " + m_name + " because could not set IP address. Error " + std::to_string(errno) + " - " + strerror(errno));
        return false;
    }

    return true;
}

bool NetworkInterface::set_netmask(uint32_t netmask) const
{
    if (m_fd < 0)
    {
        perror("socket");
        LOG_ERROR(m_logger, "socket fd is negative: " + std::to_string(m_fd));
        return false;
    }

    ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_ifru.ifru_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_ifrn.ifrn_name, m_name.c_str(), IFNAMSIZ - 1);

    constexpr size_t IP_START_INDEX = 2;

    memcpy(ifr.ifr_ifru.ifru_addr.sa_data + IP_START_INDEX, &netmask, sizeof(netmask));
    int success = ioctl(m_fd, SIOCSIFNETMASK, &ifr);
    if (success < 0)
    {
        LOG_ERROR(m_logger, "Configuring IPv4 address for interface " + m_name + " failed because could not set netmask. Error " + std::to_string(errno) + " - " + strerror(errno));
        return false;
    }

    return true;
}

uint32_t NetworkInterface::network_uint32_ipv4_from_string(const std::string& str_ip_address) const
{
    struct sockaddr_in get_addr_sa;
    if (1 != inet_pton(AF_INET, str_ip_address.c_str(), &(get_addr_sa.sin_addr)))
    {
        LOG_ERROR(m_logger, "failed to convert str address " + str_ip_address + " to ipv4 binary: " + std::to_string(errno));
        return 0;
    }

    return get_addr_sa.sin_addr.s_addr;
}
