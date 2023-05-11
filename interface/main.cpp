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
              << "keepalived_state: must be one of FAULT, BACKUP, MASTER" << std::endl;
}

int main(int argc, char* argv[])
{
    const std::string program_name = argv[0];
    if (argc < 4)
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

    auto logger = ClassFactory::Get().CreateFileLogger(program_name);
    NetworkInterface network_interface(iface);
    if (mode == "MASTER")
    {
        network_interface.deleteIpAddress(ip_addr, logger);
    }
    else
    {
        network_interface.addIpAddress(ip_addr, logger);
    }

    return 0;
}
