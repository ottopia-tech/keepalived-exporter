#include <fstream>
#include <iostream>
#include <cstring>

#include "class_factory.h"
#include "ilogger.h"
#include "network_interface.h"


void print_usage(char* program_name)
{
    std::cerr << "Usage: " << program_name << " interface ip_address keepalived_state" << std::endl
        << "interface: a required argument" << std::endl
        << "ip_address: a required argument" << std::endl
        << "keepalived_state: must be one of FAULT, BACKUP, MASTER" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        print_usage(argv[0]);
        return 1;
    }

    char* mode = argv[argc - 1];
    if (strcmp(mode, "FAULT") != 0 && strcmp(mode, "BACKUP") != 0 && strcmp(mode, "MASTER") != 0)
    {
        print_usage(argv[0]);
        return 1;
    }

    char* ip_addr = argv[argc - 2];
    char* iface = argv[argc - 3];

    auto logger = ClassFactory::Get().CreateFileLogger();
    NetworkInterface network_interface(iface);
    if (strcmp(mode, "MASTER") == 0)
    {
        network_interface.deleteIpAddress(ip_addr, logger);
    }
    else
    {
        network_interface.addIpAddress(ip_addr, 24, logger);
    }

    return 0;
}
