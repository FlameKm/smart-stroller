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
    ;;
drivers)
    scp ./drivers/build/*.ko ${BOARD_USER_NAME}@${IP_TARGET}:~
    scp -r ./app/build/test ${BOARD_USER_NAME}@${IP_TARGET}:~/app_test
    ;;
dtbos)
    scp ./drivers/build/overlay.dtbo ${BOARD_USER_NAME}@${IP_TARGET}:~
    ;;
esac
}


drives_compile() {
    make -C ./drivers;

    if [ $? -eq 0 ]; then 
        echo "driver process complete."
    else
        echo "driver process failed."
    fi
}

app_compile() {
    if [ -f "./app/build/build.ninja"]; then
        ninja -C ./app/build
    else
        cmake -B ./app/build -S ./app -GNinja -DCMAKE_CXX_COMPILER=${CROSS_COMPILE}g++ -DCMAKE_C_COMPILER=${CROSS_COMPILE}gcc
        if [ $? -eq 0 ]; then 
            ninja -C ./app/build
        else
            exit
        fi
    fi

    if [ $? -eq 0 ]; then 
        scp ./app/build/smart ${BOARD_USER_NAME}@${IP_TARGET}:~ 
        scp -r ./app/build/test ${BOARD_USER_NAME}@${IP_TARGET}:~/app_test 
    fi

    if [ $? -eq 0 ]; then 
        echo "app process complete."
    else
        echo "app process failed."
    fi
}

dts_overlay_compile() {
    dtc -I dts -O dtb -o drivers/build/overlay.dtbo drivers/overlay.dts
}

case $1 in
app)
    app_compile
    scp_file app
    ;;

drivers)
    drives_compile
    dts_overlay_compile
    scp_file drivers
    scp_file dtbos
    ;;

scp)
    scp_file all
    ;;
esac