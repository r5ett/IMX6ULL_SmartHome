# Makefile —— 一键编译出 leddrv.ko + lcdapp 两个文件
KERNELDIR := /home/book/100ask_imx6ull-sdk/Linux-4.9.88
PWD       := $(shell pwd)

# 驱动部分
obj-m += beepdrv.o

CC := $(CROSS_COMPILE)gcc

all:
	make -C $(KERNELDIR) M=$(PWD) modules          # 编译驱动
	$(CC) beepApp.c -o beepApp                       # 编译APP（自动小写）

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	rm -rf *.o *~ core .depend .*.cmd *.mod.c .tmp_versions modules.order Module.symvers
	rm -f beepApp                                           # 清理APP