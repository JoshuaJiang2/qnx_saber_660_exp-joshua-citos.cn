ROOT_DIR := $(notdir $(CURDIR))
ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

HOST_MKIFS := mkifs

SUFFIXES := .build .ifs .raw

.PHONY: all clean

all: ipl ifs-mx6x-sabreARD.raw

qnx6fsimg:
	mkqnx6fsimg qnx6fs.build joshua-qnx6fs.img
	
clean: 
	$(RM_HOST) QNX-IFS ipl-mx6x-sabreARD.bin 

ifs-mx6x-sabreARD.raw: sabreARD.build
	$(HOST_MKIFS) -vvv -r../install $(MKIFSFLAGS) $^ QNX-IFS
	
ipl:
	$(SHELL) ./mkflashimage
	