KERNELDIR = /lib/modules/`uname -r`/build
MODULES = mb_keylogger.ko
obj-m += mb_keylogger.o	

all:
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean


