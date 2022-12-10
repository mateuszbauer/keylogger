KERNELDIR=/lib/modules/`uname -r`/build
MODULE=keylogger
obj-m := $(MODULE).o
$(MODULE)-y := keylogger_main.o keyboard.o module.o sys_calls.o

all:
	make -C $(KERNELDIR) M=$(PWD) -modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean


