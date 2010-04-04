# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Common makefile for the examples.  This hase some basic variables, such as
# CC (the compiler) and some suffix rules such as .c.o.
#
# The main purpose of this makefile component is to demonstrate building
# both release and debug versions of the examples.  The debug versions
# can be loaded directly into the browser such that they can be debugged using
# standard debugging tools (gdb, KDbg, etc.)  The release versions (.nexe)
# get loaded into the browser via a web server.

.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o

OS = $(shell uname -s)
ifeq ($(OS), Darwin)
  PLATFORM = mac
  CC = nacl-gcc
  CPP = nacl-g++
  TARGET = x86
  NACL_TOOLCHAIN_DIR = compilers/host_$(PLATFORM)/target_$(TARGET)/sdk/nacl-sdk
  NACL_INCLUDE = $$SRCROOT/$(NACL_TOOLCHAIN_DIR)/nacl/include/nacl
endif
ifeq ($(OS), Linux)
  PLATFORM = linux
  CC = nacl-gcc
  CPP = nacl-g++
  TARGET = x86
  NACL_TOOLCHAIN_DIR = compilers/host_$(PLATFORM)/target_$(TARGET)/sdk/nacl-sdk
  NACL_INCLUDE = $$SRCROOT/$(NACL_TOOLCHAIN_DIR)/nacl/include/nacl
  MACHINE = $(shell uname -m)
  ifeq ($(MACHINE), x86_64)
    CC = nacl64-gcc
    CPP = nacl64-g++
    TARGET = x86
    NACL_TOOLCHAIN_DIR = compilers/host_$(PLATFORM)/target_$(TARGET)/sdk/nacl-sdk
    NACL_INCLUDE = $$SRCROOT/$(NACL_TOOLCHAIN_DIR)/nacl64/include/nacl
  endif
endif
ifneq (,$(findstring CYGWIN,$(OS)))
  PLATFORM = win
  CC = nacl-gcc
  CPP = nacl-g++
  TARGET = x86
  NACL_TOOLCHAIN_DIR = compilers/host_$(PLATFORM)/target_$(TARGET)/sdk/nacl-sdk
  NACL_INCLUDE = $$SRCROOT/$(NACL_TOOLCHAIN_DIR)/nacl/include/nacl
  MACHINE = $(shell uname -m)
  ifeq ($(MACHINE), x86_64)
    CC = nacl64-gcc
    CPP = nacl64-g++
    TARGET = x86
    NACL_TOOLCHAIN_DIR = compilers/host_$(PLATFORM)/target_$(TARGET)/sdk/nacl-sdk
    NACL_INCLUDE = $$SRCROOT/$(NACL_TOOLCHAIN_DIR)/nacl64/include/nacl
  endif
endif

OBJROOT = .
DSTROOT = .

EXTRA_CFLAGS = -Wall -Wno-long-long -pthread
ALL_CFLAGS = $(CFLAGS) $(EXTRA_CFLAGS)
ALL_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CFLAGS)
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
