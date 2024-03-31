cmake_minimum_required(VERSION 3.20)

project(aht10)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../include)


file(GLOB_RECURSE SRCS 
    main.c 
    ../../lib/*.c
    ../../src/sensor/*.c
    ../../src/hal/*.c
) 

add_executable(aht10 ${SRCS})

