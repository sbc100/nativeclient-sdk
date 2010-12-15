# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the Sine Wave Synthesizer example.

.PHONY: all clean

CCFILES =  sine_synth.cc

NACL_SDK_ROOT = ../..

INCLUDES = -I$(NACL_SDK_ROOT)
LDFLAGS = -lppruntime \
          -lppapi_cpp \
          -lgoogle_nacl_platform \
          -lgio \
          -lpthread \
          -lsrpc

all: check_variables sine_synth_x86_32.nexe sine_synth_x86_64.nexe \
	sine_synth_x86_32_dbg.nexe sine_synth_x86_64_dbg.nexe

# common.mk has rules to build .o files from .cc files.
-include ../common.mk

sine_synth_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@
	$(NACL_STRIP) $@ -o $@

sine_synth_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@
	$(NACL_STRIP) $@ -o $@

sine_synth_x86_32_dbg.nexe: $(OBJECTS_X86_32_DBG)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

sine_synth_x86_64_dbg.nexe: $(OBJECTS_X86_64_DBG)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

clean:
	-$(RM) *.obj *.o *.nexe

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: sine_synth_x86_32.nexe sine_synth_x86_64.nexe
	-$(RM) $(OBJECTS_X86_32) $(OBJECTS_X86_64)
