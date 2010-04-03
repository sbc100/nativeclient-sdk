# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the PI Generator example.

.PHONY: develop develop_linux develop_mac publish copy_files

PROGRAM_NAME = pi_generator

CCFILES = base_object.cc \
          npn_bridge.cc \
          npp_gate.cc \
          pi_generator.cc \
          pi_generator_module.cc

OBJECTS = $(patsubst %,${OBJROOT}/%,${CCFILES:%.cc=%.o})

# Mac-specific rules that install a .plugin bundle used for debugging.
PLUGIN_CONTENTS_DIR = $(PLUGIN)/Contents
PLUGIN_INFO_PLIST = $(PLUGIN_CONTENTS_DIR)/Info.plist
PLUGIN_RESOURCES_DIR = $(PLUGIN_CONTENTS_DIR)/Resources
PLUGIN_MACOS_DIR = $(PLUGIN_CONTENTS_DIR)/MacOS

-include ../common.mk

develop:
	@echo make develop in `pwd`
	$(MAKE) -$(MAKEFLAGS) copy_files
	$(MAKE) -$(MAKEFLAGS) develop_$(PLATFORM)

develop_linux:
	$(MAKE) LDFLAGS='-shared' \
	    -$(MAKEFLAGS) \
	    CFLAGS="-DXP_UNIX -Werror -fPIC" \
	    CXXFLAGS="-DXP_UNIX -Werror -fPIC" \
	    $(OBJROOT)/$(PROGRAM_NAME)
	cp $(OBJROOT)/$(PROGRAM_NAME) $(DSTROOT)/lib$(PROGRAM_NAME).so

develop_win:
	@echo Develop config does not build on Windows.

develop_mac:
	$(MAKE) LDFLAGS='-bundle -framework Foundation' \
            -$(MAKEFLAGS) \
            CFLAGS="-DXP_UNIX -Werror" \
            CXXFLAGS="-DXP_UNIX -Werror" \
            $(OBJROOT)/$(PROGRAM_NAME)
	$(MAKE) -$(MAKEFLAGS) PLUGIN=$(DSTROOT)/$(PROGRAM_NAME).plugin \
            mac_plugin_bundle

mac_plugin_bundle: $(OBJROOT)/$(PROGRAM_NAME)
	@echo Packaging the .plugin
	-rm -rf $(PLUGIN)
	mkdir -p $(PLUGIN_MACOS_DIR)
	mkdir -p $(PLUGIN_RESOURCES_DIR)
	cp $(OBJROOT)/$(PROGRAM_NAME) $(PLUGIN_MACOS_DIR)
	cp debug_support/mac/pi_generator-Info.plist \
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
	$(MAKE) -$(MAKEFLAGS) \
	    LDFLAGS="$(LIBS)" \
	    PROGRAM_NAME=$(PROGRAM_NAME).nexe \
	    CFLAGS="-DXP_UNIX -Werror" \
	    CXXFLAGS="-DXP_UNIX -Werror" \
	    $(OBJROOT)/$(PROGRAM_NAME).nexe
	cp $(OBJROOT)/$(PROGRAM_NAME).nexe $(DSTROOT)
	$(MAKE) -$(MAKEFLAGS) copy_files

copy_files:
	cp pi_generator.html $(DSTROOT)

$(OBJROOT)/$(PROGRAM_NAME): $(OBJECTS)
	$(CPP) $^ $(LDFLAGS) -o $@
