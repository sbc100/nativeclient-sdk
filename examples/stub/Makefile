# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the stub project.

.PHONY: all clean

CCFILES = stub.cc

NACL_SDK_ROOT = ../..

INCLUDES =
LDFLAGS = -lppruntime \
          -lppapi_cpp \
          -lgoogle_nacl_platform \
          -lgio \
          -lpthread \
          -lsrpc \
          $(ARCH_FLAGS)

all: check_variables stub.nmf stub_dbg.nmf

# common.mk has rules to build .o files from .cc files.
-include ../common.mk

stub.nmf: stub_x86_32.nexe stub_x86_64.nexe
	@echo "Creating stub.nmf..."
	$(PYTHON) ../generate_nmf.py --nmf $@ \
	 --x86-64 stub_x86_64.nexe --x86-32 stub_x86_32.nexe

stub_dbg.nmf: stub_x86_32_dbg.nexe stub_x86_64_dbg.nexe
	@echo "Creating stub_dbg.nmf..."
	$(PYTHON) ../generate_nmf.py --nmf $@ \
	 --x86-64 stub_x86_64_dbg.nexe --x86-32 stub_x86_32_dbg.nexe

stub_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@
	$(NACL_STRIP) $@ -o $@

stub_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@
	$(NACL_STRIP) $@ -o $@

stub_x86_32_dbg.nexe: $(OBJECTS_X86_32_DBG)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

stub_x86_64_dbg.nexe: $(OBJECTS_X86_64_DBG)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

clean:
	-$(RM) *.nmf *.o *.obj *.nexe

