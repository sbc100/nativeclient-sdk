# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the PI Generator example.

.PHONY: all clean

CCFILES = npn_bridge.cc \
          npp_gate.cc \
          pi_generator.cc \
          pi_generator_module.cc \
          scripting_bridge.cc

OBJECTS = $(CCFILES:%.cc=%.o)

NACL_SDK_ROOT = ../..

CFLAGS = -Wall -Wno-long-long -pthread -DXP_UNIX -Werror
INCLUDES = -I$(NACL_SDK_ROOT)
LDFLAGS = -lgoogle_nacl_imc \
          -lgoogle_nacl_npruntime \
          -lpthread \
          -lsrpc \
          $(ARCH_FLAGS)
OPT_FLAGS = -O2

all: check_variables pi_generator.nexe

# common.mk has rules to build .o files from .cc files.
-include ../common.mk

pi_generator.nexe: $(OBJECTS)
	$(CPP) $^ $(LDFLAGS) -o $@

clean:
	-$(RM) $(OBJECTS) pi_generator.nexe

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: pi_generator.nexe
	-$(RM) $(OBJECTS)
