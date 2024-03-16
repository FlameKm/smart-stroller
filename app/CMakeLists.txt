cmake_minimum_required(VERSION 3.20)
project(app)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set(TOOLCHAIN_DIR /home/hyc/Project/orangepi-build/toolchains/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu)
# set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-gcc)
# set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-g++)

include_directories(
    include
    include/actuator
    include/sensor
)

file(GLOB_RECURSE SRCS src/*.c)

add_executable(app ${SRCS})

add_subdirectory(unity_test)