# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the Tumbler example.

.PHONY: develop develop_linux develop_mac publish copy_files

PROGRAM_NAME = tumbler

CCFILES = cube_view.cc \
          npn_bridge.cc \
          npp_gate.cc \
          tumbler.cc \
          tumbler_module.cc \
          scripting_bridge.cc \
          shader_util.cc \
          transforms.cc
VPATH = nacl_module

APP_FILES = dragger.js \
            tumbler.html \
            tumbler.js \
            trackball.js \
            vector3.js
APP_FILES := $(addprefix application/,${APP_FILES})

OBJECTS = $(patsubst %,${OBJROOT}/%,${CCFILES:%.cc=%.o})

# Mac specific.  |PLUGIN| gets set in the call to $(MAKE) in the actions
# for the debug_mac and release_mac targets.
PLUGIN_CONTENTS_DIR = $(PLUGIN)/Contents
PLUGIN_INFO_PLIST = $(PLUGIN_CONTENTS_DIR)/Info.plist
PLUGIN_RESOURCES_DIR = $(PLUGIN_CONTENTS_DIR)/Resources
PLUGIN_MACOS_DIR = $(PLUGIN_CONTENTS_DIR)/MacOS

all: develop publish

-include ../common.mk

develop:
	@echo make develop in `pwd`
	(SRCROOT=`pwd`/../..; $(MAKE) SRCROOT=$$SRCROOT \
            CC=/usr/bin/gcc \
            CPP=/usr/bin/g++ \
            INCLUDES="-I$$SRCROOT \
                -I$$SRCROOT/third_party \
                -I$$SRCROOT/third_party/include \
                -I$$SRCROOT/third_party/npapi/bindings" \
            OBJROOT=develop/$(PLATFORM)_$(TARGET) \
            DSTROOT=../develop/$(PLATFORM)_$(TARGET) \
            OPT_FLAGS="-O0 -g3 -fPIC" develop_$(PLATFORM))
	$(MAKE) \
	    SRCROOT=`pwd`/../.. \
	    DSTROOT=../develop/$(PLATFORM)_$(TARGET) \
	    copy_files

develop_linux:
	$(MAKE) $(DSTROOT)/libtrusted_gpu.a
	$(MAKE) LDFLAGS='-shared $(ARCH_FLAGS) -L$(DSTROOT) -ltrusted_gpu' \
	    CFLAGS="-DXP_UNIX -Werror -fPIC $(ARCH_FLAGS)" \
	    CXXFLAGS="-DXP_UNIX -Werror -fPIC $(ARCH_FLAGS)" \
	    $(OBJROOT)/$(PROGRAM_NAME)
	cp $(OBJROOT)/$(PROGRAM_NAME) $(DSTROOT)/lib$(PROGRAM_NAME).so

develop_win:
	@echo Develop config does not build on Windows.

develop_mac:
	$(MAKE) $(DSTROOT)/libtrusted_gpu.a
	$(MAKE) \
	    LDFLAGS='-bundle -framework Foundation \
	        $(ARCH_FLAGS) -L$(DSTROOT) -ltrusted_gpu' \
	    CFLAGS="-DXP_UNIX -Werror" \
	    CXXFLAGS="-DXP_UNIX -Werror" \
	    $(OBJROOT)/$(PROGRAM_NAME)
	$(MAKE) PLUGIN=$(DSTROOT)/$(PROGRAM_NAME).plugin \
	    mac_plugin_bundle

mac_plugin_bundle: $(OBJROOT)/$(PROGRAM_NAME)
	@echo Packaging the .plugin
	-rm -rf $(PLUGIN)
	mkdir -p $(PLUGIN_MACOS_DIR)
	mkdir -p $(PLUGIN_RESOURCES_DIR)
	cp $(OBJROOT)/$(PROGRAM_NAME) $(PLUGIN_MACOS_DIR)
	cp debug_support/mac/tumbler-Info.plist \
	    $(PLUGIN_INFO_PLIST)
	chmod +w $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
	    -c 'Set :CFBundleExecutable $(PROGRAM_NAME)' \
	    $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
	    -c 'Set :CFBundleIdentifier com.google.$(PROGRAM_NAME)' \
	    $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
	    -c 'Set :CFBundleName $(PROGRAM_NAME)' \
	    $(PLUGIN_INFO_PLIST)
	
publish:
	@echo make publish in `pwd`
	(SRCROOT=`pwd`/../..; $(MAKE) SRCROOT=$$SRCROOT \
	    CC=$$SRCROOT/$(NACL_TOOLCHAIN_DIR)/bin/$(CC) \
	    CPP=$$SRCROOT/$(NACL_TOOLCHAIN_DIR)/bin/$(CPP) \
	    CFLAGS="-DXP_UNIX -Werror" \
	    CXXFLAGS="-DXP_UNIX -Werror" \
	    INCLUDES="-I$$SRCROOT -I$(NACL_INCLUDE)" \
	    OBJROOT=publish/$(PLATFORM)_$(TARGET) \
	    DSTROOT=../publish/$(NACL_ARCH) \
	    LDFLAGS="-lgoogle_nacl_imc \
	             -lgoogle_nacl_npruntime \
	             -lpthread \
	             -lsrpc \
	             -lgoogle_nacl_pgl \
	             -lgoogle_nacl_gpu \
	             $(ARCH_FLAGS)" \
	    OPT_FLAGS="-O2" \
	    PROGRAM_NAME=$(PROGRAM_NAME).nexe \
	    nacl_module copy_files)

nacl_module: $(OBJROOT)/$(PROGRAM_NAME)
	mkdir -p $(DSTROOT)
	cp $< $(DSTROOT)

copy_files:
	cp $(APP_FILES) $(DSTROOT)

$(OBJROOT)/$(PROGRAM_NAME): $(OBJECTS)
	$(CPP) $^ $(LDFLAGS) -o $@

$(DSTROOT)/libtrusted_gpu.a:
	(cd ../trusted_gpu; $(MAKE) -w \
            CC=/usr/bin/gcc \
            CPP=/usr/bin/g++ \
            INCLUDES="-I$$SRCROOT \
                -I$$SRCROOT/third_party \
                -I$$SRCROOT/third_party/include \
                -I$$SRCROOT/third_party/npapi/bindings" \
            OBJROOT=develop/$(PLATFORM)_$(TARGET) \
            DSTROOT=../develop/$(PLATFORM)_$(TARGET) \
            OPT_FLAGS="-O0 -g3 -fPIC" develop_$(PLATFORM))
