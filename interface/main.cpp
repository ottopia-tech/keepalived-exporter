#include <fstream>
#include <iostream>
#include <cstring>

#include "class_factory.h"
#include "ilogger.h"
#include "network_interface.h"


void print_usage(const std::string& program_name)
{
    std::cerr << "Usage: " << program_name << " interface ip_address keepalived_state" << std::endl
              << "interface: a required argument" << std::endl << "ip_address: a required argument" << std::endl
              << "netmask: an optional argument, defaults to '255.255.255.0'" << std::endl
              << "keepalived_state: must be one of FAULT, BACKUP, MASTER" << std::endl;
}

int main(int argc, char* argv[])
{
    const std::string program_name = argv[0];
    if (argc < 6)
    {
        print_usage(program_name);
        return 1;
    }

    const std::string mode = argv[argc - 1];
    if ((mode != "FAULT") && (mode != "BACKUP") && (mode != "MASTER"))
    {
        print_usage(program_name);
        return 1;
    }

    const std::string iface = argv[1];
    const std::string ip_addr = argv[2];

    std::string netmask = argv[3];
    if (argc == 6)
    {
        netmask = "255.255.255.0";
    }

    auto logger = ClassFactory::Get().CreateFileLogger(program_name);
    NetworkInterface network_interface(iface, logger);
    if (mode == "MASTER")
    {
        network_interface.DeleteIpAddress(ip_addr);
    }
    else
    {
        network_interface.AddIpAddress(ip_addr, netmask);
    }

    return 0;
}
