# Smart Stroller

## Board 

**board info**: orangpi zero3 ---1GB --- H618

**kernel version**:  linux6.1

**uboot version**: u-boot v2021.07

**toolchain**: aarch64-linux-gnu-gcc 11.2

source from https://github.com/orangepi-xunlong/orangepi-build.git

## Folder Description

**doc**: documents and information

**soft**: app and drives

## Compile

Please configure `run.sh`, contain `CROSS_COMPILE`, `BOARD_USER_NAME`, `IP_TARGET` etc.

The above kernel configuration is not supported here.

You need some tools for complite code, there is `cmake`, `ninja`, `make`. You can install it using apt of ubuntu or debian.

If you configuration environment is complete, then let me start

```bash
./run.sh app
```

```bash
./run.sh drivers
```

This will scp `output` to path `home` of the board. 

```bash
./run.sh scp
```