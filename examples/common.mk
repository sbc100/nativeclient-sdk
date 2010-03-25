# Copyright 2010, The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.

# Common makefile for the examples.  This hase some basic variables, such as
# CC (the compiler) and some suffix rules such as .c.o.
#
# The main purpose of this makefile component is to demonstrate building
# both release and debug versions of the esamples.  The debug versions
# can be loaded directly into the browser such that they can be debugged using
# standard debugging tools (gdb, KDbg, etc.)  The release versions (.nexe)
# get loaded into the browser via a web server.

.SUFFIXES: .c .cc .cpp .o

CC = /usr/bin/gcc
CPP = /usr/bin/g++
EXTRA_CFLAGS = -Wall -Wno-long-long -pthread
ALL_CFLAGS = $(CFLAGS) $(EXTRA_CFLAGS)
ALL_CXXFLAGS = $(CXXFLAGS) $(EXTRA_CFLAGS)
ALL_OPT_FLAGS = $(OPT_FLAGS)

.c.o:
	$(CC) $(ALL_CFLAGS) $(INCLUDES) $(ALL_OPT_FLAGS) -c -o $(OBJROOT)/$@ $<

.cpp.o:
.cc.o:
	$(CPP) $(ALL_CXXFLAGS) $(INCLUDES) $(ALL_OPT_FLAGS) -c -o $(OBJROOT)/$@ $<
