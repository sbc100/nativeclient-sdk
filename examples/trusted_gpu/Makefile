# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the PI Generator example.

.PHONY: debug release

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

-include ../common.mk

debug:
	@echo make -$(MAKEFLAGS) debug in `pwd`
	$(MAKE) -$(MAKEFLAGS) debug_$(PLATFORM)

debug_win:
	@echo Debug config does not build on Windows.

debug_mac debug_linux:
	$(MAKE) -$(MAKEFLAGS) \
	    CFLAGS="-DXP_UNIX -DTRUSTED_GPU_LIBRARY_BUILD -DOMIT_DLOG_AND_DCHECK" \
	    CXXFLAGS="-DXP_UNIX -DTRUSTED_GPU_LIBRARY_BUILD -DOMIT_DLOG_AND_DCHECK" \
	    $(DSTROOT)/lib$(LIBRARY_NAME).a

release:
	@echo trusted_gpu does not make release

$(DSTROOT)/lib$(LIBRARY_NAME).a: $(OBJECTS)
	ar -r -cs $@ $?
