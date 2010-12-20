# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the geturl example.

.PHONY: all clean

CCFILES = geturl.cc \
          geturl_handler.cc

NACL_SDK_ROOT = ../..

INCLUDES = -I$(NACL_SDK_ROOT)
LDFLAGS = -lppruntime \
          -lppapi_cpp \
          -lgoogle_nacl_platform \
          -lgio \
          -lpthread \
          -lsrpc \
          $(ARCH_FLAGS)

all: check_variables geturl.nmf geturl_dbg.nmf

# common.mk has rules to build .o files from .cc files.
-include ../common.mk

geturl.nmf: geturl_x86_32.nexe geturl_x86_64.nexe
	@echo "Creating geturl.nmf..."
	@echo > geturl.nmf \
	'{\n\
	  "nexes": {\n\
	    "x86-64": "geturl_x86_64.nexe",\n\
	    "x86-32": "geturl_x86_32.nexe",\n\
	  }\n\
	}\n'

geturl_dbg.nmf: geturl_x86_32_dbg.nexe geturl_x86_64_dbg.nexe
	@echo "Creating geturl_dbg.nmf..."
	@echo > geturl_dbg.nmf \
	'{\n\
	  "nexes": {\n\
	    "x86-64": "geturl_x86_64_dbg.nexe",\n\
	    "x86-32": "geturl_x86_32_dbg.nexe",\n\
	  }\n\
	}\n'

geturl_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@
	$(NACL_STRIP) $@ -o $@

geturl_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@
	$(NACL_STRIP) $@ -o $@

geturl_x86_32_dbg.nexe: $(OBJECTS_X86_32_DBG)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

geturl_x86_64_dbg.nexe: $(OBJECTS_X86_64_DBG)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

clean:
	-$(RM) *.nmf *.o *.obj *.nexe

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: geturl.nmf
	-$(RM) $(OBJECTS_X86_32) $(OBJECTS_X86_64)
