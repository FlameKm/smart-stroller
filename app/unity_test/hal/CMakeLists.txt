cmake_minimum_required(VERSION 3.20)

project(gpio)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SRCS  
    gpio.c
    ../../lib/*.c
    ../../src/hal/hal_gpio.c
) 

add_executable(gpio ${SRCS})
