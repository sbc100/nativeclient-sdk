# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Makefile for the Pepper 3D example.

CCFILES = cube_view.cc \
          npn_bridge.cc \
          npp_bridge.cc \
          pepper_3d.cc \
          pepper_module.cc \
          scripting_bridge.cc \
          shader_util.cc \
          transforms.cc
VPATH = nacl_module

APP_FILES = application/dragger.js \
            application/pepper_3d.html \
            application/pepper_3d.js \
            application/trackball.js \
            application/vector3.js

OBJECTS = $(CCFILES:.cc=.o)

PROGRAM_NAME = pepper_3d

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
	$(MAKE) LDFLAGS='-bundle -framework Foundation -L. -ltrusted_gpu' \
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
			-o $(PLUGIN_RESOURCES_DIR)/pepper_3d.rsrc \
			debug_support/mac/pepper_3d.r -useDF
	cp $(OBJROOT)/$(PROGRAM_NAME) $(PLUGIN_MACOS_DIR)
	cp debug_support/mac/pepper_3d-Info.plist \
			$(PLUGIN_INFO_PLIST)
	chmod +w $(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
			-c 'Set :CFBundleExecutable pepper_3d' \
			$(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
			-c 'Set :CFBundleIdentifier com.google.pepper_3d' \
			$(PLUGIN_INFO_PLIST)
	/usr/libexec/PlistBuddy \
			-c 'Set :CFBundleName pepper_3d' \
			$(PLUGIN_INFO_PLIST)
	
release:
	@echo make release in `pwd`
	$(MAKE) $(MAKEFLAGS) publish
	$(MAKE) $(MAKEFLAGS) \
	    LDFLAGS="$(LIBS) -lgoogle_nacl_pgl -lgoogle_nacl_gpu" \
			PROGRAM_NAME=$(PROGRAM_NAME).nexe \
			CFLAGS=-DXP_UNIX -Werror \
			CXXFLAGS=-DXP_UNIX -Werror \
			$(PROGRAM_NAME).nexe
	cp $(OBJROOT)/$(PROGRAM_NAME).nexe $(DSTROOT)

publish:
	cp $(APP_FILES) $(DSTROOT)

$(PROGRAM_NAME): $(OBJECTS)
	(cd $(OBJROOT); $(CPP) $^ $(LDFLAGS) -o $@)