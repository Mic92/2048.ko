ifneq ($(KERNELRELEASE),) 
obj-m += 2048.o
2048-y += module.o game.o

else
KERNELDIR ?= /lib/modules/$(shell uname -r)/build 
PWD     := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD)  modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

endif

