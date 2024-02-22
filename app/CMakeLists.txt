cmake_minimum_required(VERSION 3.20)
project(app)

set(TOOLCHAIN_DIR /home/hyc/Project/orangepi-build/toolchains/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu)
set(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-g++)

set(CMAKE_CXX_STANDARD 14)

add_executable(app
        src/main.cpp)
