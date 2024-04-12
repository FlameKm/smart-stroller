cmake_minimum_required(VERSION 3.20)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../include)


file(GLOB_RECURSE PUBLIC_FILE 
    ../../src/comm/tcpserver.c
    ../../lib/*.c
) 

project(tcp)
add_executable(tcp ${PUBLIC_FILE} tcp.c)