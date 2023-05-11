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

NetworkInterface::NetworkInterface(const std::string& name) :
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

bool NetworkInterface::addIpAddress(const std::string& ipAddress, std::shared_ptr<ILogger> logger)
{
    if (m_fd < 0)
    {
        std::string message = "socket fd is negative: " + std::to_string(m_fd);
        LOG_ERROR(logger, message);
        perror("socket");
        return false;
    }
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    inet_aton(ipAddress.c_str(), &sin.sin_addr);

    ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, m_name.c_str(), IFNAMSIZ - 1);

    memcpy(&ifr.ifr_addr, &sin, sizeof(sockaddr));
    int ret_val = ioctl(m_fd, SIOCSIFADDR, &ifr);
    if (ret_val < 0)
    {
        std::string message = "ioctl SIOCSIFADDR, ret_val: " + std::to_string(ret_val);
        LOG_ERROR(logger, message);
        return false;
    }

    std::string message = "successfully added ip: " + ipAddress + " on interface: " + m_name;
    LOG_INFO(logger, message);
    return true;
}

bool NetworkInterface::deleteIpAddress(const std::string& ipAddress, std::shared_ptr<ILogger> logger)
{
    if (m_fd < 0)
    {
        std::string message = "socket fd is negative: " + std::to_string(m_fd);
        LOG_ERROR(logger, message);
        perror("socket");
        return false;
    }

    InterfaceAddresses interfaceAddresses(m_name);
    // Check if the IP address exists on the interface
    if (!interfaceAddresses.ipAddressExists(ipAddress))
    {
        std::string message = "Error: IP address " + ipAddress + " not found on interface " + m_name;
        LOG_ERROR(logger, message);
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
        std::string message = "ioctl SIOCSIFADDR, ret_val: " + std::to_string(ret_val);
        LOG_ERROR(logger, message);
        return false;
    }

    std::string message = "successfully deleted ip: " + ipAddress + " on interface: " + m_name;
    LOG_INFO(logger, message);
    return true;
}
