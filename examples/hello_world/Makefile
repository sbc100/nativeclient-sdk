# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the Hello World example.

CCFILES = hello_world_module.cc hello_world.cc npn_bridge.cc npp_gate.cc

OBJECTS = $(CCFILES:%.cc=%.o)

SDK_ROOT = ../..

CFLAGS = -Wall -Wno-long-long -pthread -DXP_UNIX -Werror
INCLUDES =
LDFLAGS = -lgoogle_nacl_imc \
          -lgoogle_nacl_npruntime \
          -lpthread \
          -lsrpc \
          $(ARCH_FLAGS)
OPT_FLAGS = -O2

all: hello_world.nexe

# common.mk has rules to build .o files from .cc files.
-include ../common.mk

hello_world.nexe: $(OBJECTS)
	$(CPP) $^ $(LDFLAGS) -o $@

clean:
	-$(RM) $(OBJECTS) hello_world.nexe

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: hello_world.nexe
	-$(RM) $(OBJECTS)
