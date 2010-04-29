# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the development support library.

.PHONY: develop publish

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

VPATH = $(SRCROOT)/third_party/include/base \
        $(SRCROOT)/third_party/gpu/command_buffer/common \
        $(SRCROOT)/third_party/gpu/command_buffer/client \
        $(SRCROOT)/third_party/gpu/pgl

LIBRARY_NAME = trusted_gpu
OBJECTS = $(patsubst %,${OBJROOT}/%,${CCFILES:%.cc=%.o})

all: develop

-include ../develop_common.mk

develop:
	@echo make -$(MAKEFLAGS) develop in `pwd`
	$(MAKE) -f develop.mk -w -$(MAKEFLAGS) develop_$(PLATFORM)

develop_linux:
	$(MAKE) -f develop.mk -w -$(MAKEFLAGS) \
	    CFLAGS="$(ARCH_FLAGS) \
	        -DXP_UNIX \
	        -DTRUSTED_GPU_LIBRARY_BUILD \
	        -DOMIT_DLOG_AND_DCHECK" \
	    CXXFLAGS="$(ARCH_FLAGS) \
	        -DXP_UNIX \
	        -DTRUSTED_GPU_LIBRARY_BUILD \
	        -DOMIT_DLOG_AND_DCHECK" \
	    $(DSTROOT)/lib$(LIBRARY_NAME).a

$(DSTROOT)/lib$(LIBRARY_NAME).a: $(OBJECTS)
	mkdir -p $(DSTROOT)
	ar -r -cs $@ $?
