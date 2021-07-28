cmake_minimum_required(VERSION 3.19)
project(captive-netns-helper C)

set(CMAKE_C_STANDARD 99)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr/local/bin)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

add_executable(captive-netns-helper main.c)
install(TARGETS captive-netns-helper
        RUNTIME DESTINATION bin)