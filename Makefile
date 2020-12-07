TARGET := synch
SRC := synch gpio_helpers
SRC_DIR := src
INCL_DIR := inc
BUILD_DIR := build

INSTALL_PATH := /srv/nfs/busybox/test_modules

export KDIR := /home/$(USER)/repos/linux-stable
export PATH := /opt/gcc-arm-8.3-2019.03-x86_64-arm-eabi/bin:$(PATH)
export CROSS_COMPILE := 'arm-eabi-'
export ARCH := arm


ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m   := $(addsuffix .o, $(TARGET))
synch-objs := $(addsuffix .o, $(addprefix $(SRC_DIR)/, $(SRC)))
ccflags-y := -g -I$(PWD)/$(INCL_DIR)
else

# normal makefile
KDIR ?= /lib/modules/`uname -r`/build

.PHONY: default clean install

default: $(addsuffix .c, $(OBJS))
	$(MAKE) -C $(KDIR) M=$$PWD modules
	cp $(TARGET).ko $(TARGET).ko.unstripped
	$(CROSS_COMPILE)strip -g $(TARGET).ko

clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean

install: $(INSTALL_PATH)
	sudo cp ./$(addsuffix .ko, $(TARGET)) $<

$(INSTALL_PATH):
	sudo mkdir $@

%.i %.s : %.c
	$(ENV_CROSS) \
	$(MAKE) -C $(KDIR) M=$$PWD $@

endif
