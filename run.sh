IP_TARGET="192.168.1.78"
BOARD_USER_NAME="root"
ARCH="arm64"
CROSS_COMPILE="/home/hyc/Project/orangepi-build/toolchains/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-"

SCRIPT_DIR=$(cd $(dirname $0); pwd)
cd ${SCRIPT_DIR}

scp_file() {
case $1 in
app) 
    scp ./app/build/smart ${BOARD_USER_NAME}@${IP_TARGET}:~
    scp -r ./app/build/app_test ${BOARD_USER_NAME}@${IP_TARGET}:~/
    ;;
drivers)
    scp ./drivers/build/*.ko ${BOARD_USER_NAME}@${IP_TARGET}:~
    ;;
dtbos)
    scp ./drivers/build/overlay.dtbo ${BOARD_USER_NAME}@${IP_TARGET}:~
    ;;
esac
}


drives_compile() {
    make -C ./drivers 
    cp ./drivers/src/*.ko ./drivers/build

    if [ $? -eq 0 ]; then 
        echo "\033[32mdriver process complete.\033[0m"
        return 0
    else
        echo "\033[31mdriver process failed.\033[0m"
        return 1
    fi
}

app_compile() {
    if [ -f "./app/build/build.ninja"]; then
        ninja -C ./app/build
    else
        rm -rf ./app/build
        cmake -B ./app/build -S ./app -GNinja -DCMAKE_CXX_COMPILER=${CROSS_COMPILE}g++ -DCMAKE_C_COMPILER=${CROSS_COMPILE}gcc
        if [ $? -eq 0 ]; then 
            ninja -C ./app/build
        else
            return 0
        fi
    fi

    if [ $? -eq 0 ]; then 
        echo "\033[32app process complete.\033[0m"
        return 0
    else
        echo "\033[33app process failed.\033[0m"
        return 1
    fi
}

dts_overlay_compile() {
    dtc -I dts -O dtb -o drivers/build/overlay.dtbo drivers/overlay.dts
    return $?
}

case $1 in
app)
    app_compile && 
    scp_file app
    ;;

drivers)
    rm ./drivers/build/*
    drives_compile &&
    dts_overlay_compile &&
    scp_file drivers
    scp_file dtbos
    ;;

scp)
    scp_file all
    ;;
esac