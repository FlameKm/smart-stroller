cmake_minimum_required(VERSION 3.20)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../include)


file(GLOB_RECURSE PUBLIC_FILE 
    ../../lib/*.c
    ../../src/sensor/*.c
    ../../src/hal/*.c
) 

project(aht10)
add_executable(aht10 ${PUBLIC_FILE} aht10.c)
project(sw18015)
add_executable(sw18015 ${PUBLIC_FILE} sw18015.c)
project(hlk2411s)
add_executable(hlk2411s ${PUBLIC_FILE} hlk2411s.c)
project(vl6180)
add_executable(vl6180 ${PUBLIC_FILE} vl6180.c)