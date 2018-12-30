# In case of cross compiling, use the following lines:
#
# CCPREFIX=arm-linux-gnueabihf-
# KERNELDIR=/home/frobino/isomount/Linux_tools/altera_soc/kernel/linux-socfpga
#
# cross:
# 	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${KERNELDIR} M=$(PWD) modules

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	obj-m := my_device.o
# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD  := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
bear:
	bear $(MAKE) -C $(KERNELDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
endif
