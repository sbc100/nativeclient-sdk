# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the Tumbler example.

.PHONY: debug debug_linux debug_mac release publish

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
	$(MAKE) \
	    LDFLAGS='-bundle -framework Foundation -L$(DSTROOT) -ltrusted_gpu' \
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
	cp debug_support/mac/tumbler-Info.plist \
	    $(PLUGIN_INFO_PLIST)
	chmod +w $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
	    -c 'Set :CFBundleExecutable tumbler' \
	    $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
	    -c 'Set :CFBundleIdentifier com.google.tumbler' \
	    $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
	    -c 'Set :CFBundleName tumbler' \
	    $(PLUGIN_INFO_PLIST)
	
release:
	@echo make release in `pwd`
	$(MAKE) -$(MAKEFLAGS) publish
	$(MAKE) -$(MAKEFLAGS) \
	    LDFLAGS="$(LIBS) -lgoogle_nacl_pgl -lgoogle_nacl_gpu" \
	    PROGRAM_NAME=$(PROGRAM_NAME).nexe \
	    CFLAGS="-DXP_UNIX -Werror" \
	    CXXFLAGS="-DXP_UNIX -Werror" \
	    $(OBJROOT)/$(PROGRAM_NAME).nexe
	cp $(OBJROOT)/$(PROGRAM_NAME).nexe $(DSTROOT)

publish:
	cp $(APP_FILES) $(DSTROOT)

$(OBJROOT)/$(PROGRAM_NAME): $(OBJECTS)
	$(CPP) $^ $(LDFLAGS) -o $@
