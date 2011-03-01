# Copyright 2011, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the Tumbler example.

.PHONY: all clean

CCFILES = cube.cc \
          opengl_context.cc \
          script_array.cc \
          scripting_bridge.cc \
          shader_util.cc \
          transforms.cc \
          tumbler.cc \
          tumbler_module.cc
HFILES = callback.h \
         cube.h \
         opengl_context.h \
         opengl_context_ptrs.h \
         script_array.h \
         scripting_bridge.h \
         shader_util.h \
         transforms.h \
         tumbler.h

RELEASE_MODULES = tumbler_x86_32.nexe tumbler_x86_64.nexe
DEBUG_MODULES = tumbler_x86_32_dbg.nexe tumbler_x86_64_dbg.nexe
APPLICATION_FILES = dragger.js \
                    tumbler.nmf \
                    tumbler.html \
                    tumbler.js \
                    trackball.js \
                    vector3.js \
                    $(RELEASE_MODULES)

NACL_SDK_ROOT ?= ../..
EXAMPLE_ROOT ?= ..
INSTALL_DIR ?= $$HOME/Sites/tumbler

INCLUDES = -I$(NACL_SDK_ROOT)
LDFLAGS = -lppruntime \
          -lppapi_cpp \
          -lplatform \
          -lgio \
          -lpthread \
          -lsrpc \
          -lppapi_gles2 \
          $(ARCH_FLAGS)

all: check_variables tumbler.nmf tumbler_dbg.nmf

# common.mk has rules to build .o files from .cc files.
-include $(EXAMPLE_ROOT)/common.mk

tumbler.nmf: $(RELEASE_MODULES)
	@echo "Creating tumbler.nmf..."
	$(PYTHON) $(EXAMPLE_ROOT)/generate_nmf.py --nmf $@ \
	 --x86-64 tumbler_x86_64.nexe --x86-32 tumbler_x86_32.nexe

tumbler_dbg.nmf: $(DEBUG_MODULES)
	@echo "Creating tumbler_dbg.nmf..."
	$(PYTHON) $(EXAMPLE_ROOT)/generate_nmf.py --nmf $@ \
	 --x86-64 tumbler_x86_64_dbg.nexe \
	 --x86-32 tumbler_x86_32_dbg.nexe

tumbler_x86_32.nexe: $(OBJECTS_X86_32)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@
	$(NACL_STRIP) $@ -o $@

tumbler_x86_64.nexe: $(OBJECTS_X86_64)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@
	$(NACL_STRIP) $@ -o $@

tumbler_x86_32_dbg.nexe: $(OBJECTS_X86_32_DBG)
	$(CPP) $^ $(LDFLAGS) -m32 -o $@

tumbler_x86_64_dbg.nexe: $(OBJECTS_X86_64_DBG)
	$(CPP) $^ $(LDFLAGS) -m64 -o $@

clean:
	-$(RM) *.o *.obj *.nexe

# This target is used by the SDK build system to produce a pre-built version
# of the .nexe.  You do not need to call this target.
install_prebuilt: tumbler.nmf
	-$(RM) $(OBJECTS_X86_32) $(OBJECTS_X86_64)

install: $(APPLICATION_FILES) $(INSTALL_DIR)
	install $(APPLICATION_FILES) $(INSTALL_DIR)

$(INSTALL_DIR):
	mkdir -p $@

