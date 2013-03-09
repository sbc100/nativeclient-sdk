# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

TARGETS = lua_level_layer.cpp LuaCocos2dExtensions.cpp

HELPER = tolua_preload.lua

TOLUA = tolua++5.1

all: $(TARGETS)

lua_level_layer.cpp: level_layer.pkg $(HELPER) post_process.py
	$(TOLUA) -L $(HELPER) -H lua_level_layer.h -o $@ $<
	./post_process.py $@

LuaCocos2dExtensions.cpp: extensions.pkg $(HELPER) post_process.py
	$(TOLUA) -L $(HELPER) -H $(@:.cpp=.h) -o $@ $<
	./post_process.py $@

clean:
	rm -f $(TARGETS)

.PHONY: clean all
