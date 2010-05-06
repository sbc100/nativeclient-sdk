# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the trusted_gpu development support library.

.PHONY: lib clean install_prebuilt

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
  CC = "cl"
  CPP = "cl"
  AR = "lib"
  AR_FLAGS =
  ARCH_FLAGS =
  OPT_FLAGS = -Od
  CFLAGS = /TP /EHsc /DOS_WIN=1 /DOMIT_LOG_AND_CHECK=1 /DUNICODE=1 /DNOMINMAX=1
  obj_out = /Fo$(subst /,\\,$(dir $(1)))
  ar_out = /OUT:$(subst /,\\,$(1))
  platform_objs = $(1:%.o=%.obj)
endif
ifeq ($(OS), $(filter $(OS), Darwin MACOS))
  PLATFORM = mac
  TARGET = x86
  CC = /usr/bin/gcc
  CPP = /usr/bin/g++
  AR = /usr/bin/ar
  AR_FLAGS = -r -cs
  OPT_FLAGS = -O0
  CFLAGS = $(ARCH_FLAGS) \
           -DXP_UNIX \
           -DTRUSTED_GPU_LIBRARY_BUILD \
           -DOMIT_DLOG_AND_DCHECK
  obj_out = -o $(1)
  ar_out = $(1)
  platform_objs = $(1)
endif
ifneq (, $(findstring Linux, $(OS)))
  PLATFORM = linux
  TARGET = x86
  CC = /usr/bin/gcc
  CPP = /usr/bin/g++
  AR = /usr/bin/ar
  AR_FLAGS = -r -cs
  OPT_FLAGS = -O0
  CFLAGS = $(ARCH_FLAGS) \
           -DXP_UNIX \
           -DTRUSTED_GPU_LIBRARY_BUILD \
           -DOMIT_DLOG_AND_DCHECK
  obj_out = -o $(1)
  ar_out = $(1)
  platform_objs = $(1)
endif

# The debug libraries make 32-bit by default.
ifeq ($(PLATFORM), win)
  WORD_SIZE ?= 32
  ARCH_FLAGS =
else
  WORD_SIZE ?= 32
  ARCH_FLAGS = -m$(WORD_SIZE)
endif

NACL_SDK_ROOT ?= ../..
OBJROOT := $(PLATFORM)_$(TARGET)_$(WORD_SIZE)
DSTROOT := ../$(PLATFORM)_$(TARGET)_$(WORD_SIZE)

INCLUDES = -I$(NACL_SDK_ROOT) \
           -I$(NACL_SDK_ROOT)/third_party \
           -I$(NACL_SDK_ROOT)/third_party/include \
           -I$(NACL_SDK_ROOT)/third_party/npapi/bindings

CCFILES = logging.cc \
          cmd_buffer_common.cc \
          gles2_cmd_format.cc \
          gles2_cmd_utils.cc \
          cmd_buffer_helper.cc \
          fenced_allocator.cc \
          gles2_c_lib.cc \
          gles2_cmd_helper.cc \
          gles2_implementation.cc \
          gles2_lib.cc \
          id_allocator.cc \
          command_buffer_pepper.cc \
          pgl.cc

VPATH = $(NACL_SDK_ROOT)/third_party/include/base \
        $(NACL_SDK_ROOT)/third_party/gpu/command_buffer/common \
        $(NACL_SDK_ROOT)/third_party/gpu/command_buffer/client \
        $(NACL_SDK_ROOT)/third_party/gpu/pgl

LIBRARY_NAME = trusted_gpu
OBJECTS = $(patsubst %,${OBJROOT}/%,${CCFILES:%.cc=%.o})

install_prebuilt: clean lib
	-rm -rf $(OBJROOT)

lib: $(OBJROOT) $(DSTROOT) $(DSTROOT)/lib$(LIBRARY_NAME).a

clean:
	-rm -rf $(DSTROOT) $(OBJROOT)

$(OBJROOT) $(DSTROOT)::
	mkdir -p $@

$(DSTROOT)/lib$(LIBRARY_NAME).a: $(OBJECTS)
	$(AR) $(AR_FLAGS) $(call ar_out,$@) $(call platform_objs,$?)

$(OBJROOT)/%.o: %.c
	$(CC) $(CFLAGS) $(ARCH_FLAGS) $(INCLUDES) $(OPT_FLAGS) -c $(call obj_out,$@) $<

$(OBJROOT)/%.o: %.cc
	$(CPP) $(CFLAGS) $(ARCH_FLAGS) $(INCLUDES) $(OPT_FLAGS) -c $(call obj_out,$@) $<

$(OBJROOT)/%.(OBJ): %.cpp
	$(CPP) $(CFLAGS) $(ARCH_FLAGS) $(INCLUDES) $(OPT_FLAGS) -c $(call obj_out,$@) $<
