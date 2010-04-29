# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Common makefile for the examples.  This has some basic variables, such as
# CC (the compiler) and some suffix rules such as .c.o.
#
# The main purpose of this makefile component is to demonstrate building
# both publish and develop versions of the examples.  The develop versions
# can be loaded directly into the browser such that they can be debugged using
# standard debugging tools (gdb, KDbg, etc.)  The publish versions (.nexe)
# get loaded into the browser via a web server.

.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o

# To make a 64-bit build, set WORD_SIZE=64 on the command line.  For example:
#   make develop WORD_SIZE=64
# Note that 64-bit builds are not supported on Mac or Linux.
ifeq ($(origin WORD_SIZE), undefined)
  ifeq (,$(findstring Intel64,$(PROCESSOR_IDENTIFIER)))
	WORD_SIZE = 32
  else
	WORD_SIZE = 64
  endif
endif
ARCH_FLAGS = -m$(WORD_SIZE)

OS = $(shell uname -s)
ifeq ($(OS), Darwin)
  PLATFORM = mac
  TARGET = x86
  NACL_ARCH = x86_$(WORD_SIZE)
  CC = nacl64-gcc
  CPP = nacl64-g++
  NACL_TOOLCHAIN_DIR = toolchain/$(PLATFORM)_$(TARGET)
  NACL_INCLUDE = $$SRCROOT/$(NACL_TOOLCHAIN_DIR)/nacl/include/nacl
endif
ifeq ($(OS), Linux)
  PLATFORM = linux
  TARGET = x86
  NACL_ARCH = x86_$(WORD_SIZE)
  CC = nacl64-gcc
  CPP = nacl64-g++
  NACL_TOOLCHAIN_DIR = toolchain/$(PLATFORM)_$(TARGET)
  NACL_INCLUDE = $$SRCROOT/$(NACL_TOOLCHAIN_DIR)/nacl/include/nacl
  # TODO(dspringer): Enable this section when Linux supports a 64-bit runtime.
  # ifeq ($(WORD_SIZE), 64)
    # NACL_INCLUDE = $$SRCROOT/$(NACL_TOOLCHAIN_DIR)/nacl64/include/nacl
  # endif
endif
ifneq (,$(findstring CYGWIN,$(OS)))
  PLATFORM = win
  TARGET = x86
  NACL_ARCH = x86_$(WORD_SIZE)
  CC = nacl64-gcc
  CPP = nacl64-g++
  NACL_TOOLCHAIN_DIR = toolchain/$(PLATFORM)_$(TARGET)
  NACL_INCLUDE = $$SRCROOT/$(NACL_TOOLCHAIN_DIR)/nacl/include/nacl
endif

OBJROOT = .
DSTROOT = .

EXTRA_CFLAGS += -Wall -Wno-long-long -pthread
ALL_CFLAGS = $(CFLAGS) $(EXTRA_CFLAGS) $(ARCH_FLAGS)
ALL_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CFLAGS) $(ARCH_FLAGS)
ALL_OPT_FLAGS = $(OPT_FLAGS)

$(OBJROOT)/%.o: %.c
	mkdir -p $(OBJROOT)
	$(CC) $(ALL_CFLAGS) $(INCLUDES) $(ALL_OPT_FLAGS) -c -o $@ $<

$(OBJROOT)/%.o: %.cc
	mkdir -p $(OBJROOT)
	$(CPP) $(ALL_CXXFLAGS) $(INCLUDES) $(ALL_OPT_FLAGS) -c -o $@ $<

$(OBJROOT)/%.o: %.cpp
	mkdir -p $(OBJROOT)
	$(CPP) $(ALL_CXXFLAGS) $(INCLUDES) $(ALL_OPT_FLAGS) -c -o $@ $<

clean::
	-rm -rf develop publish
