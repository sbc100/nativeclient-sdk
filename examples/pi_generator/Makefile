# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the PI Generator example.

.PHONY: all clean

CCFILES = pi_generator.cc pi_generator_module.cc

NACL_SDK_ROOT = ../..

INCLUDES = -I$(NACL_SDK_ROOT)
LDFLAGS = -lppruntime \
          -lppapi_cpp \
          -lplatform \
          -lgio \
          -lpthread \
          -lsrpc \
          $(ARCH_FLAGS)

all: check_variables pi_generator.nmf pi_generator_dbg.nmf

# common.mk has rules to build .o files from .cc files.
-include ../common.mk

pi_generator.nmf: pi_generator_x86_32.nexe pi_generator_x86_64.nexe
	@echo "Creating pi_generator.nmf..."
	$(PYTHON) ../generate_nmf.py --nmf $@ \
	 --x86-64 pi_generator_x86_64.nexe --x86-32 pi_generator_x86_32.nexe

pi_generator_dbg.nmf: pi_generator_x86_32_dbg.nexe pi_generator_x86_64_dbg.nexe
	@echo "Creating pi_generator_dbg.nmf..."
	$(PYTHON) ../generate_nmf.py --nmf $@ \
	 --x86-64 pi_generator_x86_64_dbg.nexe \
	 --x86-32 pi_generator_x86_32_dbg.nexe

pi_generator_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@
	$(NACL_STRIP) $@ -o $@

pi_generator_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@
	$(NACL_STRIP) $@ -o $@

pi_generator_x86_32_dbg.nexe: $(OBJECTS_X86_32_DBG)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

pi_generator_x86_64_dbg.nexe: $(OBJECTS_X86_64_DBG)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

clean:
	-$(RM) *.nmf *.o *.obj *.nexe

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: pi_generator.nmf
	-$(RM) $(OBJECTS_X86_32) $(OBJECTS_X86_64)
