# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

TARGET = lua-yaml
OUTBASE = out/lua-yaml/obj
LIB_OUT = out/lua-yaml

include $(NACL_SDK_ROOT)/tools/common.mk

LUA_YAML_ROOT = third_party/lua-yaml
COCOS2DX_ROOT = third_party/cocos2d-x

CINCLUDE = -I$(LUA_YAML_ROOT) \
  -I$(COCOS2DX_ROOT)/scripting/lua/lua \

SOURCES = \
   $(LUA_YAML_ROOT)/lyaml.c \
   $(LUA_YAML_ROOT)/api.c \
   $(LUA_YAML_ROOT)/dumper.c \
   $(LUA_YAML_ROOT)/emitter.c \
   $(LUA_YAML_ROOT)/loader.c \
   $(LUA_YAML_ROOT)/parser.c \
   $(LUA_YAML_ROOT)/reader.c \
   $(LUA_YAML_ROOT)/scanner.c \
   $(LUA_YAML_ROOT)/writer.c \
   $(LUA_YAML_ROOT)/b64.c


$(foreach src,$(SOURCES),$(eval $(call COMPILE_RULE,$(src),$(CFLAGS),$(CINCLUDE))))
$(eval $(call LIB_RULE,$(TARGET),$(SOURCES)))
