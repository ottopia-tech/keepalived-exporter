cmake_minimum_required(VERSION 3.25)
project(interface)

set(CMAKE_CXX_STANDARD 17)

list(APPEND SOURCES
        main.cpp
        class_factory.cpp
        file_logger.cpp
        network_interface.cpp
        interface_addresses.cpp
)

add_executable(interface ${SOURCES})
