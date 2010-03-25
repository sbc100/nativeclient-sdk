# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Main makefile for the Native Client SDK examples.  These targets set certain
# make variables and then call the descendant makefiles in each of the
# examples.

.PHONY: all debug release obj

EXAMPLES = npapi_helloworld npapi_pi_generator pepper_3d
DEBUG_LIBS = trusted_gpu

PLATFORM = mac
TARGET = x86-32
SRCROOT = ..
OBJROOT = .
DSTROOT = .

NACL_TOOLCHAIN_DIR = compilers/host_$(PLATFORM)/target-$(TARGET)/sdk/nacl-sdk

all: debug release

# Make debug versions of all the modules.
debug:
	$(MAKE) $(MAKEFLAGS) OBJROOT=debug/obj/$(PLATFORM)_$(TARGET) objroot
	$(MAKE) $(MAKEFLAGS) DSTROOT=debug/staging/$(PLATFORM)_$(TARGET) dstroot
	@for examp in $(DEBUG_LIBS) $(EXAMPLES) ; do \
		(cd $$examp ; SRCROOT=`pwd`/../..; $(MAKE) $(MAKEFLAGS) \
				CC=/usr/bin/gcc \
				CPP=/usr/bin/g++ \
				SRCROOT=`pwd`/../.. \
				INCLUDES="-I$$SRCROOT \
									-I$$SRCROOT/third_party \
									-I$$SRCROOT/third_party/include \
									-I$$SRCROOT/third_party/npapi/bindings" \
				PLATFORM=$(PLATFORM) \
				TARGET=$(TARGET) \
				OBJROOT=../debug/obj/$(PLATFORM)_$(TARGET) \
				DSTROOT=../debug/staging/$(PLATFORM)_$(TARGET) \
				OPT_FLAGS="-O0 -gfull" debug); \
	done

release:
	$(MAKE) $(MAKEFLAGS) OBJROOT=release/obj/$(PLATFORM)_$(TARGET) objroot
	$(MAKE) $(MAKEFLAGS) DSTROOT=release/staging/$(PLATFORM)_$(TARGET) dstroot
	@for examp in $(EXAMPLES) ; do \
		(cd $$examp ; SRCROOT=`pwd`/../..; $(MAKE) $(MAKEFLAGS) \
				CC=$$SRCROOT/$(NACL_TOOLCHAIN_DIR)/bin/nacl-gcc \
				CPP=$$SRCROOT/$(NACL_TOOLCHAIN_DIR)/bin/nacl-g++ \
				SRCROOT=`pwd`/../.. \
				INCLUDES="-I$$SRCROOT \
									-I$$SRCROOT/$(NACL_TOOLCHAIN_DIR)/nacl/include/nacl" \
				PLATFORM=$(PLATFORM) \
				TARGET=$(TARGET) \
				OBJROOT=../release/obj/$(PLATFORM)_$(TARGET) \
				DSTROOT=../release/staging/$(PLATFORM)_$(TARGET) \
				LIBS="-lgoogle_nacl_imc -lgoogle_nacl_npruntime -lpthread -lsrpc" \
				OPT_FLAGS="-O2" release); \
	done

clean:
	@rm -rf debug release

objroot:
	@if ! [ -e $(OBJROOT) ] ; then \
		echo mkdir -p $(OBJROOT); \
		mkdir -p $(OBJROOT); \
	fi

dstroot:
	@if ! [ -e $(DSTROOT) ] ; then\
		echo mkdir -p $(DSTROOT); \
		mkdir -p $(DSTROOT); \
	fi
