IP_TARGET="192.168.3.33"
BOARD_USER_NAME="orangepi"
ARCH="arm64"
CROSS_COMPILE="/opt/gcc-ubuntu-9.3.0-2020.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu-"

config() {
    export CROSS_COMPILE_YC=${CROSS_COMPILE}
}

drives_compile() {
    make -C ./drivers
    if [ $? -eq 0 ]; then 
        cp ./drivers/src/**.ko ./drivers/build
        scp ./drivers/build/* ${BOARD_USER_NAME}@${IP_TARGET}:/home/${BOARD_USER_NAME}/drivers
    fi
}

app_compile() {
    if [ -f "./app/build/build.ninja"]; then
        ninja -C ./app/build
    else
        cmake -B ./app/build -S ./app -GNinja
        if [ $? -eq 0 ]; then 
            ninja -C ./app/build
        fi
    fi

    if [ $? -eq 0 ]; then 
        scp ./app/build/app ${BOARD_USER_NAME}@${IP_TARGET}:/home/${BOARD_USER_NAME}/app
    fi
}

case $1 in
app)
    app_compile
    ;;

drivers)
    drives_compile
    ;;

config)
    config
    ;;
esac