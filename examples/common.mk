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

# Note that on Windows builds, OS has to be defined by the calling shell.
ifeq ($(origin OS), undefined)
  OS := $(shell uname -s)
endif

# To make a 64-bit build, you can set WORD_SIZE=64 on the command line.
#   make WORD_SIZE=64
# Note that 64-bit builds are not supported on Mac or Linux.
# If you are running on an x86_64 Windows computer, WORD_SIZE should be set
# for you automatically.
ifeq ($(OS), win)
  ifeq ($(origin WORD_SIZE), undefined)
    ifeq (,$(findstring Intel64,$(PROCESSOR_IDENTIFIER)))
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

ifeq ($(OS), win)
  PLATFORM = win
  TARGET = x86
  RM = CMD /C DEL /Q
endif
ifeq ($(OS), Darwin)
  PLATFORM = mac
  TARGET = x86
  RM = rm -f
endif
ifeq ($(OS), Linux)
  PLATFORM = linux
  TARGET = x86
  RM = rm -f
endif
ifneq (,$(findstring CYGWIN,$(OS)))
  PLATFORM = win
  TARGET = x86
  RM = CMD /C DEL /Q
endif

NACL_ARCH = x86_$(WORD_SIZE)

NACL_TOOLCHAIN_DIR = toolchain/$(PLATFORM)_$(TARGET)

ifeq ($(WORD_SIZE), 64)
  NACL_INCLUDE = $(SDK_ROOT)/$(NACL_TOOLCHAIN_DIR)/nacl64/include/nacl
else
  NACL_INCLUDE = $(SDK_ROOT)/$(NACL_TOOLCHAIN_DIR)/nacl/include/nacl
endif

CC = $(SDK_ROOT)/$(NACL_TOOLCHAIN_DIR)/bin/nacl64-gcc
CPP = $(SDK_ROOT)/$(NACL_TOOLCHAIN_DIR)/bin/nacl64-g++

ifeq ($(OS), win)
  CC := $(subst /,\,$(CC))
  CPP := $(subst /,\,$(CPP))
endif

%.o: %.c
	$(CC) $(CFLAGS) $(ARCH_FLAGS) $(INCLUDES) $(OPT_FLAGS) -c -o $@ $<

%.o: %.cc
	$(CPP) $(CFLAGS) $(ARCH_FLAGS) $(INCLUDES) $(OPT_FLAGS) -c -o $@ $<

%.o: %.cpp
	$(CPP) $(CFLAGS) $(ARCH_FLAGS) $(INCLUDES) $(OPT_FLAGS) -c -o $@ $<
