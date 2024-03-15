TOOLCHAIN_DIR=/home/hyc/Project/orangepi-build/toolchains/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu
CMAKE_CXX_COMPILER=${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-g++
CMAKE_C_COMPILER=${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-gcc 

cmake -GNinja -B build -S .
ninja -C build