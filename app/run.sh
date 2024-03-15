TOOLCHAIN_DIR=/home/hyc/Project/orangepi-build/toolchains/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu
CXX_COMPILER=${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-g++
C_COMPILER=${TOOLCHAIN_DIR}/bin/aarch64-none-linux-gnu-gcc 

rm -rf build/*
cmake -GNinja -B build -S . -DCMAKE_CXX_COMPILER=${CXX_COMPILER} -DCMAKE_C_COMPILER=${C_COMPILER}
ninja -C build