# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

#
# GNU Make based build file.  For details on GNU Make see:
#   http://www.gnu.org/software/make/manual/make.html
#
#

#
# Default configuration
#
# By default we will build a Debug configuration using the GCC newlib toolcahin
# to override this, specify TOOLCHAIN=newlib|glibc or CONFIG=Debug|Release on
# the make command-line or in this file prior to including common.mk.  The
# toolchain we use by default will be the first valid one listed
VALID_TOOLCHAINS:=newlib
TOOLCHAIN:=newlib

OUTBASE:=out

#
# Target Name
#
# The base name of the final NEXE, also the name of the NMF file containing
# the mapping between architecture and actual NEXE.
#
TARGET:=nacltoons

COCOS_ROOT = ../third_party/cocos2d-x

#
# List of sources to compile
#
SOURCES := src/main.cc \
	   src/app_delegate.cc \
	   src/frontend.cc \
	   src/gameplay_scene.cc \
	   src/physics_layer.cc \
	   bindings/LuaBox2D.cpp \
	   bindings/lua_physics_layer.cpp \
	   bindings/LuaCocos2dExtensions.cpp \
	   $(COCOS_ROOT)/samples/Cpp/TestCpp/Classes/Box2DTestBed/GLES-Render.cpp \
	   $(COCOS_ROOT)/extensions/physics_nodes/CCPhysicsDebugNode.cpp \
	   $(COCOS_ROOT)/extensions/physics_nodes/CCPhysicsSprite.cpp \
	   $(COCOS_ROOT)/scripting/lua/cocos2dx_support/CCLuaEngine.cpp \
	   $(COCOS_ROOT)/scripting/lua/cocos2dx_support/CCLuaStack.cpp \
	   $(COCOS_ROOT)/scripting/lua/cocos2dx_support/Cocos2dxLuaLoader.cpp \
	   $(COCOS_ROOT)/scripting/lua/cocos2dx_support/LuaCocos2d.cpp \
	   $(COCOS_ROOT)/scripting/lua/cocos2dx_support/tolua_fix.c


PAGE:=$(OUTBASE)/publish/index.html

#
# Get pepper directory for toolchain and includes.
#
# If NACL_SDK_ROOT is not set, then assume it can be found relative to
# to this Makefile.
#
ifndef NACL_SDK_ROOT
  $(error NACL_SDK_ROOT not set)
endif

NACL_SDK_VERSION_MIN := 27.186236
include $(NACL_SDK_ROOT)/tools/common.mk

# In recent SDK versions defining NACL_SDK_VERSION_MIN is enough to trigger and error
# but older SDKs didn't know about this so we check for older versions that don't
# support --sdk-version
SDK_VERSION := $(shell $(GETOS) --sdk-version)
ifndef SDK_VERSION
  $(error You need a more recent version of the NaCl SDK (>= $(NACL_SDK_VERSION_MIN)))
endif

CFLAGS += -DCOCOS2D_DEBUG -DCC_ENABLE_BOX2D_INTEGRATION
CFLAGS += -Wall -Wno-unknown-pragmas

COCOS2DX_PATH = $(OUTBASE)/cocos2dx

# By default we pull naclports from the installed SDK, but we can
# build and use them locally by defining LOCAL_PORTS
ifdef LOCAL_PORTS
NACLPORTS_ROOT = $(OUTBASE)/naclports
else
NACLPORTS_ROOT = $(NACL_SDK_ROOT)/ports
endif

CINCLUDE= \
  -Isrc \
  -Ibindings \
  -I$(COCOS2DX_PATH) \
  -I$(COCOS2DX_PATH)/cocoa \
  -I$(COCOS2DX_PATH)/external \
  -I$(COCOS2DX_PATH)/include \
  -I$(COCOS2DX_PATH)/platform/nacl \
  -I$(COCOS2DX_PATH)/kazmath/include \
  -I$(NACL_SDK_ROOT)/include \
  -I$(NACLPORTS_ROOT)/include \
  -I../third_party/cocos2d-x/external \
  -I../third_party/cocos2d-x/extensions \
  -I../third_party/cocos2d-x/samples/Cpp/TestCpp/Classes/Box2DTestBed

LIB_PATHS += $(COCOS2DX_PATH)/lib
LIB_PATHS += $(NACLPORTS_ROOT)/lib

#
# List of libraries to link against.  Unlike some tools, the GCC and LLVM
# based tools require libraries to be specified in the correct order.  The
# order should be symbol reference followed by symbol definition, with direct
# sources to the link (object files) are left most.  In this case:
#    hello_world -> ppapi_main -> ppapi_cpp -> ppapi -> pthread -> libc
# Notice that libc is implied and come last through standard compiler/link
# switches.
#
# We break this list down into two parts, the set we need to rebuild (DEPS)
# and the set we do not.  This example does not havea any additional library
# dependencies.
#
DEPS=
SOUNDLIBS=cocosdenshion alut openal vorbisfile vorbis ogg
LIBS=$(DEPS) lua cocos2d $(SOUNDLIBS) freetype box2d xml2 png12 jpeg tiff webp
LIBS+=nacl_io ppapi_gles2 ppapi ppapi_cpp z


#
# Use the library dependency macro for each dependency
#
$(foreach dep,$(DEPS),$(eval $(call DEPEND_RULE,$(dep))))

#
# Use the compile macro for each source.
#
$(foreach src,$(SOURCES),$(eval $(call COMPILE_RULE,$(src),$(CFLAGS),$(CINCLUDE))))

#
# Use the link macro for this target on the list of sources.
#
$(eval $(call LINK_RULE,$(TARGET),$(SOURCES),$(LIBS),$(DEPS)))

#
# Specify the NMF to be created with no additional arugments.
#
$(eval $(call NMF_RULE,$(TARGET),))

#
# Add extra dependacies so that our final targets get rebuilt whenever
# cocos2dx or its dependencies change.
#
$(OUTDIR)/$(TARGET)_x86_64.nexe: \
	$(COCOS2DX_PATH)/lib/newlib_x86_64/$(CONFIG)/libcocos2d.a \
	$(COCOS2DX_PATH)/lib/newlib_x86_64/$(CONFIG)/liblua.a \
	$(COCOS2DX_PATH)/lib/newlib_x86_64/$(CONFIG)/libbox2d.a

$(OUTDIR)/$(TARGET)_arm.nexe: \
	$(COCOS2DX_PATH)/lib/newlib_arm/$(CONFIG)/libcocos2d.a \
	$(COCOS2DX_PATH)/lib/newlib_arm/$(CONFIG)/liblua.a \
	$(COCOS2DX_PATH)/lib/newlib_arm/$(CONFIG)/libbox2d.a

$(OUTDIR)/$(TARGET)_x86_32.nexe: \
	$(COCOS2DX_PATH)/lib/newlib_x86_32/$(CONFIG)/libcocos2d.a \
	$(COCOS2DX_PATH)/lib/newlib_x86_32/$(CONFIG)/liblua.a \
	$(COCOS2DX_PATH)/lib/newlib_x86_32/$(CONFIG)/libbox2d.a
