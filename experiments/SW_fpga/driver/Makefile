#
# Author: Ruben Rodriguez
# This Makefile is designed to compile the module to be loded onto a Ultrascale running 5.15.19-xilinx-v2022.1

# export ARCH=armhf
# export CROSS_COMPILE=armhf-linux-gnu-
CC = $(CROSS_COMPILE)gcc
# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
 	obj-m := seqdriver.o
# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD  := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
endif
