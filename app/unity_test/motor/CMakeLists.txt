cmake_minimum_required(VERSION 3.20)

project(motor)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../include)

add_executable(motor main.c ../../src/actuator/motor.c ../../lib/log/log.c)
