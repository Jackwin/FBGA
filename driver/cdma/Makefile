KERN_DIR = /lib/modules/`uname -r`/build
ccflags-y := -I$(PWD)/inc
TARGET := cdma_drv
obj-m := $(TARGET).o
$(TARGET)-objs := drv/cdma_drv.o drv/dma_transf.o

all:
	make -C $(KERN_DIR) M=`pwd` modules
clean:
	make -C $(KERN_DIR) M=`pwd` modules clean
	rm -rf modules.order
