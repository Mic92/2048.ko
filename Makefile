DEBUG=n
KVERSION=$(shell uname -r)

ifeq ($(DEBUG),y)
	ccflags-y += -DDEBUG
endif

obj-m += 2048.o
2048-y += module.o game.o

all: 
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean: 
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
