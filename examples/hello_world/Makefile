# Copyright (c) 2011, The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Makefile for the Hello World example.

.PHONY: all clean

CCFILES = hello_world.cc helper_functions.cc
HFILES = helper_functions.h

NACL_SDK_ROOT = ../..

INCLUDES = -I$(NACL_SDK_ROOT)
LDFLAGS = -lppruntime \
          -lppapi_cpp \
          -lplatform \
          -lgio \
          -lpthread \
          -lsrpc \
          $(ARCH_FLAGS)

all: check_variables hello_world.nmf hello_world_dbg.nmf

# common.mk has rules to build .o files from .cc files.
-include ../common.mk

hello_world.nmf: hello_world_x86_32.nexe hello_world_x86_64.nexe
	@echo "Creating hello_world.nmf..."
	$(PYTHON) ../generate_nmf.py --nmf $@ \
	 --x86-64 hello_world_x86_64.nexe --x86-32 hello_world_x86_32.nexe

hello_world_dbg.nmf: hello_world_x86_32_dbg.nexe hello_world_x86_64_dbg.nexe
	@echo "Creating hello_world_dbg.nmf..."
	$(PYTHON) ../generate_nmf.py --nmf $@ \
	 --x86-64 hello_world_x86_64_dbg.nexe \
	 --x86-32 hello_world_x86_32_dbg.nexe

hello_world_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@
	$(NACL_STRIP) $@ -o $@

hello_world_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@
	$(NACL_STRIP) $@ -o $@

hello_world_x86_32_dbg.nexe: $(OBJECTS_X86_32_DBG)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

hello_world_x86_64_dbg.nexe: $(OBJECTS_X86_64_DBG)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

test_helper_functions_x86_32_dbg.nexe: test_helper_functions_x86_32_dbg.o \
                                       helper_functions_x86_32_dbg.o
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

test_helper_functions_x86_64_dbg.nexe: test_helper_functions_x86_64_dbg.o \
                                       helper_functions_x86_64_dbg.o
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

# Run the 32-bit version of the unit test with nacl-sel_ldr.
test32: test_helper_functions_x86_32_dbg.nexe
	$(NACL_SEL_LDR32) test_helper_functions_x86_32_dbg.nexe

# Run the 64-bit version of the unit test with nacl64-sel_ldr.
test64: test_helper_functions_x86_64_dbg.nexe
	$(NACL_SEL_LDR64) test_helper_functions_x86_64_dbg.nexe

# Run both the 32-bit and the 64-bit version of the tests.  Note that this is
# not included in the 'all' target because 32-bit platforms won't support the
# 64-bit test.
test: test32 test64

clean:
	-$(RM) *.nmf *.o *.nexe *.obj

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: hello_world.nmf
	-$(RM) $(OBJECTS_X86_32) $(OBJECTS_X86_64)
