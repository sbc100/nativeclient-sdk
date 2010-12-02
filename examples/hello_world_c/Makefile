# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the Hello World example.

.PHONY: all clean

CFILES = hello_world.c

NACL_SDK_ROOT = ../..

CFLAGS = -Wall -Wno-long-long -pthread -DXP_UNIX -Werror
INCLUDES =
LDFLAGS = -lppruntime \
          -lpthread \
          -lsrpc \
          $(ARCH_FLAGS)
OPT_FLAGS = -O2

all: check_variables hello_world_x86_32.nexe hello_world_x86_64.nexe

# common.mk has rules to build .o files from .cc files.
-include ../common.mk

# have to link with g++: the implementation of the PPAPI proxy
# is a C++ implementation

hello_world_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

hello_world_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

clean:
	-$(RM) $(OBJECTS_X86_32) $(OBJECTS_X86_64) \
	    hello_world_x86_32.nexe hello_world_x86_64.nexe

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: hello_world_x86_32.nexe hello_world_x86_64.nexe
	-$(RM) $(OBJECTS_X86_32) $(OBJECTS_X86_64)
