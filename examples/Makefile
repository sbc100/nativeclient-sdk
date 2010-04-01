# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Main makefile for the Native Client SDK examples.  These targets set certain
# make variables and then call the descendant makefiles in each of the
# examples.

.PHONY: all debug release clean

EXAMPLES = npapi_helloworld npapi_pi_generator pepper_3d
DEBUG_LIBS = trusted_gpu

OS = $(shell uname -s)
ifeq ($(OS), Darwin)
  PLATFORM = mac
  # There is only support for 32-bit Native Client toolchain on mac.
  CC = nacl-gcc
  CPP = nacl-g++
  TARGET = x86-32
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
SRCROOT = ..
OBJROOT = .
DSTROOT = .

all: debug release

# Make debug versions of all the modules.
debug:
	mkdir -p debug/$(PLATFORM)_$(TARGET)
	@for examp in $(DEBUG_LIBS) $(EXAMPLES) ; do \
	    (cd $$examp ; SRCROOT=`pwd`/../..; $(MAKE) -w \
	        CC=/usr/bin/gcc \
		CPP=/usr/bin/g++ \
		SRCROOT=`pwd`/../.. \
		INCLUDES="-I$$SRCROOT \
		    -I$$SRCROOT/third_party \
		    -I$$SRCROOT/third_party/include \
		    -I$$SRCROOT/third_party/npapi/bindings" \
		PLATFORM=$(PLATFORM) \
		TARGET=$(TARGET) \
		OBJROOT=debug/$(PLATFORM)_$(TARGET) \
		DSTROOT=../debug/$(PLATFORM)_$(TARGET) \
		OPT_FLAGS="-O0 -g3 -fPIC" debug); \
	done

release:
	mkdir -p release/$(PLATFORM)_$(TARGET)
	@for examp in $(EXAMPLES) ; do \
	    (cd $$examp ; SRCROOT=`pwd`/../..; $(MAKE) -w \
	        CC=$$SRCROOT/$(NACL_TOOLCHAIN_DIR)/bin/$(CC) \
	        CPP=$$SRCROOT/$(NACL_TOOLCHAIN_DIR)/bin/$(CPP) \
	        SRCROOT=`pwd`/../.. \
	        INCLUDES="-I$$SRCROOT -I$(NACL_INCLUDE)" \
	        PLATFORM=$(PLATFORM) \
	        TARGET=$(TARGET) \
	        OBJROOT=release/$(PLATFORM)_$(TARGET) \
	        DSTROOT=../release/$(PLATFORM)_$(TARGET) \
	        LIBS="-lgoogle_nacl_imc -lgoogle_nacl_npruntime -lpthread -lsrpc" \
	        OPT_FLAGS="-O2" release); \
	done

clean:
	-rm -rf debug release
	@for examp in $(DEBUG_LIBS) $(EXAMPLES) ; do \
	    (cd $$examp ; $(MAKE) clean); \
	done
