# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile that builds a release version of the Tumbler Native Client module.
# This makefile is meant to be called from another makefile or an IDE (such as
# Xcode). There are some required make variables that are expeced to be set by
# the caller:
#   PLATFORM = <mac, win, linux>  The host platform.  This variable is used
#       to build up other variables that point at the correct NaCl toolchain.
#   TARGET = <x86, x86-32>  The target architecture.  This variable is used
#       in conjunction with |PLATFORM| to build up other variables that point
#       at the correct NaCl toolchain.
#   SDK_ROOT = <path>  The path to the NaCl SDK root.

.PHONY: all release

PROGRAM_NAME = tumbler

NACL_CC = nacl-gcc
NACL_CPP = nacl-g++
NACL_TOOLCHAIN_DIR = compilers/host_$(PLATFORM)/target_$(TARGET)/sdk/nacl-sdk
NACL_INCLUDE = $(SDK_ROOT)/$(NACL_TOOLCHAIN_DIR)/nacl/include/nacl

CC = $(SDK_ROOT)/$(NACL_TOOLCHAIN_DIR)/bin/$(NACL_CC)
CPP = $(SDK_ROOT)/$(NACL_TOOLCHAIN_DIR)/bin/$(NACL_CPP)
INCLUDES = -I$(SDK_ROOT) -I$(NACL_INCLUDE)
OBJROOT = release/$(PLATFORM)_$(TARGET)
DSTROOT = ../release/$(PLATFORM)_$(TARGET)
LIBS = -lgoogle_nacl_imc \
       -lgoogle_nacl_npruntime \
       -lpthread \
       -lsrpc \
       -lgoogle_nacl_pgl \
       -lgoogle_nacl_gpu
OPT_FLAGS = "-O2"

LDFLAGS = $(LIBS)
CFLAGS = -DXP_UNIX -Werror
CXXFLAGS = -DXP_UNIX -Werror

CCFILES = cube_view.cc \
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
            tumber.js \
            trackball.js \
            vector3.js
APP_FILES := $(addprefix application/,${APP_FILES})

OBJECTS = $(patsubst %,${OBJROOT}/%,${CCFILES:%.cc=%.o})

-include ../common.mk

all release:
	$(MAKE) -$(MAKEFLAGS) publish
	$(MAKE) -$(MAKEFLAGS) $(OBJROOT)/$(PROGRAM_NAME).nexe
	cp $(OBJROOT)/$(PROGRAM_NAME).nexe $(DSTROOT)

publish:
	cp $(APP_FILES) $(DSTROOT)

$(OBJROOT)/$(PROGRAM_NAME).nexe:
	$(CPP) $^ $(LDFLAGS) -o $@
