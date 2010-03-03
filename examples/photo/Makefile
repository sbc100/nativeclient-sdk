# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# simple GNU Makefile for photo demo
# (Linux, Mac & Cygwin)
#
# Please see native_client/common/Makefile.mk for details
# TODO: remove dependency on native_client directory

CCFLAGS=-Wall -Werror

# if nacl target, copy nexe to photo.nexe for photo.html to launch
ifeq (nacl,$(filter nacl,$(MAKECMDGOALS)))
  CCFLAGS += -Xlinker --wrap -Xlinker read
  CCFLAGS += -Xlinker --wrap -Xlinker open
  CCFLAGS += -Xlinker --wrap -Xlinker close
  CCFLAGS += -Xlinker --wrap -Xlinker lseek
  POST_BUILD = cp $(EXE_NAME) photo.nexe
else
  $(warning This demo requires 'nacl' as a build target.)
  $(warning For example, 'make release nacl')
  $(warning See native_client/common/Makefile.mk for all build options.)
  $(error Unable to continue)
endif

ifeq (release,$(filter release,$(MAKECMDGOALS)))
  OPT=-O3 -march=pentium4 -mfpmath=sse -msse3
  OPT+=-funroll-loops -fomit-frame-pointer
endif

NAME:=photo
FILES:=photo.cc surface.cc nacl_file.cc
LIBS:=-ljpeg

CLIENT:=$(filter-out *%,$(subst /native_client,/native_client *,$(CURDIR)))
include $(CLIENT)/common/Makefile.mk

