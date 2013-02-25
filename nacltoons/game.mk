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

OUTBASE:=out

#
# Target Name
#
# The base name of the final NEXE, also the name of the NMF file containing
# the mapping between architecture and actual NEXE.
#
TARGET:=nacltoons

#
# List of sources to compile
#
SOURCES := src/main.cc \
	   src/app_delegate.cc \
	   src/frontend.cc \
	   src/gameplay_scene.cc \
	   src/physics_layer.cc \
	   ../third_party/cocos2d-x/samples/Cpp/TestCpp/Classes/Box2DTestBed/GLES-Render.cpp \
	   ../third_party/cocos2d-x/extensions/physics_nodes/CCPhysicsDebugNode.cpp \
	   ../third_party/cocos2d-x/extensions/physics_nodes/CCPhysicsSprite.cpp

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

include $(NACL_SDK_ROOT)/tools/common.mk

ifdef NACL_MOUNTS
CFLAGS += -DOLD_NACL_MOUNTS
endif
CFLAGS += -DCOCOS2D_DEBUG -DCC_ENABLE_BOX2D_INTEGRATION
COCOS2DX_PATH = $(OUTBASE)/cocos2dx
NACLPORTS_PATH = $(OUTBASE)/naclports
CINCLUDE= \
  -IClasses \
  -I$(COCOS2DX_PATH) \
  -I$(COCOS2DX_PATH)/cocoa \
  -I$(COCOS2DX_PATH)/external \
  -I$(COCOS2DX_PATH)/include \
  -I$(COCOS2DX_PATH)/platform/nacl \
  -I$(COCOS2DX_PATH)/kazmath/include \
  -I$(NACL_SDK_ROOT)/include \
  -I$(NACLPORTS_PATH)/include \
  -I../third_party/cocos2d-x/external \
  -I../third_party/cocos2d-x/extensions \
  -I../third_party/cocos2d-x/samples/Cpp/TestCpp/Classes/Box2DTestBed

LIB_PATHS += $(COCOS2DX_PATH)/lib
LIB_PATHS += $(NACLPORTS_PATH)/lib

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
LIBS=$(DEPS) cocos2d $(SOUNDLIBS) freetype box2d xml2 png12 jpeg tiff
ifdef NACL_MOUNTS
LIBS+=nacl-mounts
else
LIBS+=nacl_io
endif
LIBS+=ppapi_gles2 ppapi ppapi_cpp z


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

