IP_TARGET="192.168.1.78"
BOARD_USER_NAME="root"
ARCH="arm64"
CROSS_COMPILE="/home/hyc/Project/orangepi-build/toolchains/gcc-arm-11.2-2022.02-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-"

scp_file() {
    scp ./drivers/build/*.ko ${BOARD_USER_NAME}@${IP_TARGET}:~
    scp ./app/build/smart ${BOARD_USER_NAME}@${IP_TARGET}:~
    scp -r ./app/build/test ${BOARD_USER_NAME}@${IP_TARGET}:~/app_test
    if [ $? -eq 0 ]; then 
        echo "scp process complete."
    else
        echo "scp process failed."
    fi
}


drives_compile() {
    make -C ./drivers
    if [ $? -eq 0 ]; then 
        cp ./drivers/src/**.ko ./drivers/build
        scp ./drivers/build/*.ko ${BOARD_USER_NAME}@${IP_TARGET}:~ > /dev/null
    fi

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
        scp ./app/build/smart ${BOARD_USER_NAME}@${IP_TARGET}:~ > /dev/null
        scp -r ./app/build/test ${BOARD_USER_NAME}@${IP_TARGET}:~/app_test > /dev/null
    fi

    if [ $? -eq 0 ]; then 
        echo "app process complete."
    else
        echo "app process failed."
    fi
}

case $1 in
app)
    app_compile
    ;;

drivers)
    drives_compile
    ;;

scp)
    scp_file
    ;;
esac