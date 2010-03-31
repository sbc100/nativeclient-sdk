# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the PI Generator example.

.PHONY: debug debug_linux debug_mac release publish

PROGRAM_NAME = npapi_pi_generator

CCFILES = base_object.cc \
          npn_bridge.cc \
          npp_gate.cc \
          pi_generator_module.cc \
          plugin.cc

OBJECTS = $(patsubst %,${OBJROOT}/%,${CCFILES:%.cc=%.o})

# Mac-specific rules that install a .plugin bundle used for debugging.
PLUGIN_CONTENTS_DIR = $(PLUGIN)/Contents
PLUGIN_INFO_PLIST = $(PLUGIN_CONTENTS_DIR)/Info.plist
PLUGIN_RESOURCES_DIR = $(PLUGIN_CONTENTS_DIR)/Resources
PLUGIN_MACOS_DIR = $(PLUGIN_CONTENTS_DIR)/MacOS

-include ../common.mk

debug:
	@echo make debug in `pwd`
	$(MAKE) -$(MAKEFLAGS) publish
	$(MAKE) -$(MAKEFLAGS) debug_$(PLATFORM)

debug_linux:
	$(MAKE) LDFLAGS='-shared' \
	    -$(MAKEFLAGS) \
	    CFLAGS="-DXP_UNIX -Werror -fPIC" \
	    CXXFLAGS="-DXP_UNIX -Werror -fPIC" \
	    $(OBJROOT)/$(PROGRAM_NAME)
	cp $(OBJROOT)/$(PROGRAM_NAME) $(DSTROOT)/lib$(PROGRAM_NAME).so

debug_win:
	@echo Debug config does not build on Windows.

debug_mac:
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
	/Developer/Tools/Rez \
            -o $(PLUGIN_RESOURCES_DIR)/npapi_pi_generator.rsrc \
            debug_support/mac/npapi_pi_generator.r -useDF
	cp $(OBJROOT)/$(PROGRAM_NAME) $(PLUGIN_MACOS_DIR)
	cp debug_support/mac/npapi_pi_generator-Info.plist \
            $(PLUGIN_INFO_PLIST)
	chmod +w $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
	    -c 'Set :CFBundleExecutable npapi_pi_generator' \
	    $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
	    -c 'Set :CFBundleIdentifier com.google.npapi_pi_generator' \
	    $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
	    -c 'Set :CFBundleName npapi_pi_generator' \
	    $(PLUGIN_INFO_PLIST)
	
release:
	@echo make release in `pwd`
	$(MAKE) -$(MAKEFLAGS) publish
	$(MAKE) -$(MAKEFLAGS) \
	    LDFLAGS="$(LIBS)" \
	    PROGRAM_NAME=$(PROGRAM_NAME).nexe \
	    CFLAGS="-DXP_UNIX -Werror" \
	    CXXFLAGS="-DXP_UNIX -Werror" \
	    $(OBJROOT)/$(PROGRAM_NAME).nexe
	cp $(OBJROOT)/$(PROGRAM_NAME).nexe $(DSTROOT)

publish:
	cp npapi_pi_generator.html $(DSTROOT)

$(OBJROOT)/$(PROGRAM_NAME): $(OBJECTS)
	$(CPP) $^ $(LDFLAGS) -o $@
