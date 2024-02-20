KERN_DIR = /home/hyc/Project/orangepi-build/kernel/orange-pi-6.1-sun50iw9

# INCLUDE_DIR = ../inc/

# CFLAGS = -Wall -I$(INCLUDE_DIR)

all: 
	make -C $(KERN_DIR) M=`pwd` modules 

clean:
	make -C $(KERN_DIR) M=`pwd` modules clean

obj-m += src/motor.o