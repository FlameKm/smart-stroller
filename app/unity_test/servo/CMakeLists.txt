cmake_minimum_required(VERSION 3.20)

project(servo)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../include)

add_executable(servo servo.c ../../src/actuator/servo.c ../../lib/log/log.c)
