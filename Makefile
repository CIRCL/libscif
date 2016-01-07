# Copyright 2010-2013 Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2,
# as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# Disclaimer: The codes contained in these modules may be specific to
# the Intel Software Development Platform codenamed Knights Ferry,
# and the Intel product codenamed Knights Corner, and are not backward
# compatible with other Intel products. Additionally, Intel will NOT
# support the codes or instruction set in future products.
#
# Intel offers no warranty of any kind regarding the code. This code is
# licensed on an "AS IS" basis and Intel is not obligated to provide
# any support, assistance, installation, training, or other services
# of any kind. Intel is also not obligated to provide any updates,
# enhancements or extensions. Intel specifically disclaims any warranty
# of merchantability, non-infringement, fitness for any particular
# purpose, and any other warranty.
#
# Further, Intel disclaims all liability of any kind, including but
# not limited to liability for infringement of any proprietary rights,
# relating to the use of the code, even if Intel is notified of the
# possibility of such liability. Except as expressly stated in an Intel
# license agreement provided with this code and agreed upon with Intel,
# no license, express or implied, by estoppel or otherwise, to any
# intellectual property rights is granted herein.

include mpss-metadata.mk

CFLAGS ?= -g
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
CC = $(CROSS_COMPILE)gcc
CPP = $(CC) -E
AR = $(CROSS_COMPILE)ar
NM = $(CROSS_COMPILE)nm
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
INSTALL = install
INSTALL_d = $(INSTALL) -d
INSTALL_x = $(INSTALL)
INSTALL_f = $(INSTALL) -m 644

prefix = /usr/local
exec_prefix = $(prefix)
datarootdir = $(prefix)/share
includedir = $(prefix)/include
docdir = $(datarootdir)/doc/libscif-$(libscif_major).$(libscif_minor)
pdfdir = $(docdir)
libdir = $(exec_prefix)/lib
mandir = $(datarootdir)/man
man3dir = $(mandir)/man3
man9dir = $(mandir)/man9

TARGET_ARCH := $(shell $(CC) -dumpmachine | sed -n 's/.*\b\([lk]1om\)\b.*/\1/p')

ALL_CFLAGS_k1om = -D_MIC_SCIF_
ALL_CFLAGS_l1om = -D_MIC_SCIF_
ALL_CFLAGS += $(CFLAGS) $(ALL_CFLAGS_$(TARGET_ARCH)) -I$(CURDIR)

# Makes it easy to inject "-Wall -Werror" from the environment
ALL_CFLAGS += $(USERWARNFLAGS)

libscif_major := 0
libscif_minor := 0.1
libscif_dev := libscif.so
libscif_abi := libscif.so.$(libscif_major)
libscif_all := libscif.so.$(libscif_major).$(libscif_minor)

.PHONY: all lib pdf install install-pdf docs

all: $(libscif_dev) docs

libscif.map: libscif.cfg scif_api.c
	gen-symver-map $< $@ -- $(filter-out $<, $^) -- $(CC) $(ALL_CFLAGS)

$(libscif_dev): scif_api.c libscif.map
	$(CC) $(ALL_CFLAGS) -fpic -shared $< $(MPSS_METADATA_BRAND) -o $@ \
		-Wl,-soname,$(libscif_abi) \
		-Wl,--version-script=libscif.map

pdf:
	@echo create PDF...

A2X = a2x
SCIFMANDIR = man
UMANDIR = $(SCIFMANDIR)/man_3
KMANDIR = $(SCIFMANDIR)/man_9
UMANSRCS = $(wildcard $(UMANDIR)/*.txt)
KMANSRCS = $(wildcard $(KMANDIR)/*.txt)
UMANPAGES = $(UMANSRCS:.txt=.3)
KMANPAGES = $(KMANSRCS:.txt=.9)

docs: $(UMANPAGES) $(KMANPAGES)
%.3:%.txt
	$(A2X) --doctype manpage -D $(UMANDIR) --format  manpage --attribute=scif_user $<
%.9:%.txt
	$(A2X) --doctype manpage -D $(KMANDIR) --format  manpage $<
install:
	$(INSTALL_d) $(DESTDIR)$(libdir)
	$(INSTALL_x) $(libscif_dev) $(DESTDIR)$(libdir)/$(libscif_all)
	ln -s $(libscif_all) $(DESTDIR)$(libdir)/$(libscif_abi)
	ln -s $(libscif_all) $(DESTDIR)$(libdir)/$(libscif_dev)
	$(INSTALL_d) $(DESTDIR)$(includedir)
	$(INSTALL_f) scif.h $(DESTDIR)$(includedir)/scif.h
	$(INSTALL_d) $(DESTDIR)$(man3dir)
	$(INSTALL_d) $(DESTDIR)$(man9dir)
	$(INSTALL_f) $(UMANDIR)/*.3 $(DESTDIR)$(man3dir)
	$(INSTALL_f) $(KMANDIR)/*.9 $(DESTDIR)$(man9dir)

install-pdf:
	@echo install PDF...
clean:
	rm -f $(UMANDIR)/*.3
	rm -f $(UMANDIR)/*.xml
	rm -f $(KMANDIR)/*.9
	rm -f $(KMANDIR)/*.xml
