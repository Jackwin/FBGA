KERN_DIR = /lib/modules/`uname -r`/build
ccflags-y := -I$(PWD)/inc
TARGET := gpio_drv
obj-m := $(TARGET).o
$(TARGET)-objs := drv/gpio_drv.o

all:
	make -C $(KERN_DIR) M=`pwd` modules
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
