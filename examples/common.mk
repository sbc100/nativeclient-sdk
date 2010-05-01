# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Common makefile for the examples.  This has some basic variables, such as
# CC (the compiler) and some suffix rules such as .c.o.
#
# The main purpose of this makefile component is to demonstrate building a
# Native Client module (.nexe)

.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o

ifeq ($(origin OS), undefined)
  ifeq ($(shell uname -s), Darwin)
    OS=Darwin
  else
    OS=$(shell uname -o)
  endif
endif

ifeq ($(OS), $(filter $(OS), Windows_NT Cygwin))
  PLATFORM = win
  TARGET = x86
endif
ifeq ($(OS), $(filter $(OS), Darwin MACOS))
  PLATFORM = mac
  TARGET = x86
endif
ifeq ($(OS), Linux)
  PLATFORM = linux
  TARGET = x86
endif

# To make a 64-bit build, you can set WORD_SIZE=64 on the command line.
#   make WORD_SIZE=64
# Note that 64-bit builds are not supported on Mac.
# If you are running on an x86_64 Windows computer, WORD_SIZE should be set
# for you automatically.
ifeq ($(PLATFORM), win)
  ifeq ($(origin WORD_SIZE), undefined)
    ifneq (, $(filter Intel64 AMD64, $(PROCESSOR_IDENTIFIER)))
      WORD_SIZE = 64
    else
      WORD_SIZE = 32
    endif
  endif
else
  WORD_SIZE ?= 32
endif

ARCH_FLAGS = -m$(WORD_SIZE)
SDK_ROOT ?= .

NACL_TOOLCHAIN_DIR = toolchain/$(PLATFORM)_$(TARGET)

CC = $(SDK_ROOT)/$(NACL_TOOLCHAIN_DIR)/bin/nacl-gcc
CPP = $(SDK_ROOT)/$(NACL_TOOLCHAIN_DIR)/bin/nacl-g++

%.o: %.c
	$(CC) $(CFLAGS) $(ARCH_FLAGS) $(INCLUDES) $(OPT_FLAGS) -c -o $@ $<

%.o: %.cc
	$(CPP) $(CFLAGS) $(ARCH_FLAGS) $(INCLUDES) $(OPT_FLAGS) -c -o $@ $<

%.o: %.cpp
	$(CPP) $(CFLAGS) $(ARCH_FLAGS) $(INCLUDES) $(OPT_FLAGS) -c -o $@ $<
