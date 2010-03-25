# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the Hello World example.

CCFILES = npapi_helloworld.cc npp_gate.cc

OBJECTS = $(CCFILES:.cc=.o)

PROGRAM_NAME = npapi_helloworld

# Mac specific.  |PLUGIN| gets set in the call to $(MAKE) in the actions
# for the debug_mac and release_mac targets.
PLUGIN_CONTENTS_DIR = $(PLUGIN)/Contents
PLUGIN_INFO_PLIST = $(PLUGIN_CONTENTS_DIR)/Info.plist
PLUGIN_RESOURCES_DIR = $(PLUGIN_CONTENTS_DIR)/Resources
PLUGIN_MACOS_DIR = $(PLUGIN_CONTENTS_DIR)/MacOS

-include ../common.mk

debug:
	@echo make debug in `pwd`
	$(MAKE) $(MAKEFLAGS) publish
	$(MAKE) $(MAKEFLAGS) debug_$(PLATFORM)

debug_mac:
	$(MAKE) LDFLAGS='-bundle -framework Foundation' \
			$(MAKEFLAGS) \
			CFLAGS=-DXP_UNIX -Werror \
			CXXFLAGS=-DXP_UNIX -Werror \
			$(PROGRAM_NAME)
	$(MAKE) $(MAKEFLAGS) PLUGIN=$(DSTROOT)/$(PROGRAM_NAME).plugin \
			mac_plugin_bundle

mac_plugin_bundle: $(OBJROOT)/$(PROGRAM_NAME)
	@echo Packaging the .plugin
	-rm -rf $(PLUGIN)
	mkdir -p $(PLUGIN_MACOS_DIR)
	mkdir -p $(PLUGIN_RESOURCES_DIR)
	/Developer/Tools/Rez \
			-o $(PLUGIN_RESOURCES_DIR)/npapi_helloworld.rsrc \
			debug_support/mac/npapi_helloworld.r -useDF
	cp $(OBJROOT)/$(PROGRAM_NAME) $(PLUGIN_MACOS_DIR)
	cp debug_support/mac/npapi_helloworld-Info.plist \
			$(PLUGIN_INFO_PLIST)
	chmod +w $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
			-c 'Set :CFBundleExecutable npapi_helloworld' \
			$(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
			-c 'Set :CFBundleIdentifier com.google.npapi_helloworld' \
			$(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
			-c 'Set :CFBundleName npapi_helloworld' \
			$(PLUGIN_INFO_PLIST)
	
release:
	@echo make release in `pwd`
	$(MAKE) $(MAKEFLAGS) publish
	$(MAKE) $(MAKEFLAGS) \
	    LDFLAGS="$(LIBS)" \
			PROGRAM_NAME=$(PROGRAM_NAME).nexe \
			CFLAGS=-DXP_UNIX -Werror \
			CXXFLAGS=-DXP_UNIX -Werror \
			$(PROGRAM_NAME).nexe
	cp $(OBJROOT)/$(PROGRAM_NAME).nexe $(DSTROOT)

publish:
	cp npapi_helloworld.html $(DSTROOT)
	cp nacl_js_lib.js $(DSTROOT)

$(PROGRAM_NAME): $(OBJECTS)
	(cd $(OBJROOT); $(CPP) $^ $(LDFLAGS) -o $@)