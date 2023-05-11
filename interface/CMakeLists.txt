cmake_minimum_required(VERSION 3.24)
project(interface)

set(CMAKE_CXX_STANDARD 17)

list(APPEND SOURCES
        main.cpp
        class_factory.cpp
        file_logger.cpp
        network_interface.cpp
        interface_addresses.cpp
)

message(STATUS "PROJECT: ${CMAKE_PROJECT_NAME}")
add_executable(${CMAKE_PROJECT_NAME} ${SOURCES})
install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION /opt/ottopia/keepalived-exporter)
