cmake_minimum_required(VERSION 3.20)


include_directories(${CMAKE_CURRENT_SOURCE_DIR} ../../include)

project(motor)
add_executable(motor motor.c ../../src/actuator/motor.c ../../lib/log/log.c)
project(servo)
add_executable(servo servo.c ../../src/actuator/servo.c ../../lib/log/log.c)
project(chassis)
add_executable(chassis 
    chassis.c
    ../../src/actuator/motor.c 
    ../../src/actuator/servo.c
    ../../src/actuator/chassis.c
    ../../lib/log/log.c
)