# This is the top-level Makefile for all source packages.
# It makes all the code in the "src" directory, then installs it
# in the "install" directory, then makes the images in
# the images directory (if present).

#==============qconfig.mk =============================================#
# source /home/joshua/QNX/BSP/qnx700/qnxsdp-env.sh
#QNX_HOST=/home/joshua/QNX/BSP/qnx700/host/linux/x86_64
#QNX_TARGET=/home/joshua/QNX/BSP/qnx700/target/qnx7
#MAKEFLAGS=-I/home/joshua/QNX/BSP/qnx700/target/qnx7/usr/include

#root@ubuntu:/home/joshua/QNX/BSP/qnx700# find ./ -name qconfig.mk
#./target/qnx7/usr/include/qconfig.mk
#./target/qnx7/usr/include/mk/qconfig.mk

#通过执行env脚本，makefile可以找到需要的sdk

#qconfig.mk  再次include 同目录下的 ./mk/qconfig.mk,以下为mk/qconfig.mk的内容

#QCONFIG_BASEDIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
#include $(QNX_HOST)/etc/host.mk
#include $(QCONFIG_BASEDIR)/qconf-$(HOST_OS).mk

#基本上，通过导入qconfig.mk 来设定编译过程中会用到的tool的PATH
#joshua@citos.com
#=====================================================================#


ROOT_DIR := $(notdir $(CURDIR))
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)
unexport ROOT_DIR

.PHONY: all install clean links make_links dummy images prebuilt binaries

# Expands to a single newline character
define NEWLINE


endef

SUFFIXES := .mk

all:  install links $(if $(wildcard images/*),images)
	@echo done

subdirs:=$(subst /Makefile,,$(wildcard */[Mm]akefile))

clean:
	$(foreach dir,$(subdirs), $(MAKE) -C$(dir) clean $(NEWLINE))
	-$(RM_HOST) -r install/*

#首先做 prebuild target，将预先编译好的文件拷贝到install目录
install: $(if $(wildcard prebuilt/*),prebuilt)
#hinstall 会递归将子目录下的头文件拷贝到install 目录
	$(MAKE) -Csrc hinstall
#递归编译src code
	$(MAKE) -Csrc

#
# Have to invoke "make_links" target because the first make expands
# the $(wildcard ...) too soon - we might not have copied things into
# the "install" tree yet.
#
#直接使用images目录下的build文件即可
links:
#	$(MAKE) make_links

make_links:
	$(foreach file,$(wildcard install/*/boot/build/*),cd images;$(LN_HOST) ../$(file) $(notdir $(file));cd ..; )

images:
	$(MAKE) -Cimages

qnx6img:
	$(MAKE) -Cimages qnx6fsimg
	
prebuilt:
	cp -rf prebuilt/* install
#graphics.conf 需要修改，不再使用prebuild里的原始版本
#	cp -f ./images/graphics.conf ./install/armle-v7/usr/lib/graphics/iMX6X/graphics.conf

