#!/bin/bash
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

SCRIPT_DIR=$(dirname ${BASH_SOURCE[0]})

export LUA_PATH="${SCRIPT_DIR}/data/res/?.lua"
export LUA_CPATH="${SCRIPT_DIR}/third_party/lua-yaml/yaml.so"

lua $*
