# 检查驱动是否已加载
if lsmod | grep -q motor_spwm; then
    echo "Unloading motor-spwm driver..."
    rmmod motor-spwm
fi

if lsmod | grep -q servo; then
    echo "Unloading servo driver..."
    rmmod servo
fi

if lsmod | grep -q sw18015; then
    echo "Unloading sw18015 driver..."
    rmmod sw18015
fi

# 加载驱动
insmod drivers/motor-spwm.ko && echo "motor-spwm driver loaded."
insmod drivers/servo.ko && echo "servo driver loaded."
insmod drivers/sw18015.ko && echo "sw18015 driver loaded."