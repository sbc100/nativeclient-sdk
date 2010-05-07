# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the Tumbler example.

.PHONY: all clean

CCFILES = cube.cc \
          npn_bridge.cc \
          npp_gate.cc \
          tumbler.cc \
          tumbler_module.cc \
          scripting_bridge.cc \
          shader_util.cc \
          transforms.cc
VPATH = nacl_module

APP_FILES = dragger.js \
            tumbler.html \
            tumbler.js \
            trackball.js \
            vector3.js

OBJECTS = $(CCFILES:%.cc=%.o)

NACL_SDK_ROOT = ../..

CFLAGS = -Wall -Wno-long-long -pthread -DXP_UNIX -Werror
INCLUDES = -I$(NACL_SDK_ROOT)
LDFLAGS = -lgoogle_nacl_imc \
          -lgoogle_nacl_npruntime \
          -lpthread \
          -lsrpc \
          -lgoogle_nacl_pgl \
          -lgoogle_nacl_gpu \
          $(ARCH_FLAGS)
OPT_FLAGS = -O2

all: check_variables tumbler.nexe

# common.mk has rules to build .o files from .cc files.
-include ../common.mk

tumbler.nexe: $(OBJECTS)
	$(CPP) $^ $(LDFLAGS) -o $@

clean:
	-$(RM) $(OBJECTS) tumbler.nexe

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: tumbler.nexe
	-$(RM) $(OBJECTS)
