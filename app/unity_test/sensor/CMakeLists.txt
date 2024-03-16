cmake_minimum_required(VERSION 3.20)

project(sensor)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(sensor main.c)
