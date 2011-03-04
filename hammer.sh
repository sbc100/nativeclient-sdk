#!/bin/bash
#
# Copyright (c) 2010 The Native Client SDK Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

CURR_DIR=`pwd -P`
export NACL_SDK_ROOT=$CURR_DIR
export SCONS_DIR=$CURR_DIR/third_party/scons/scons-local
# We have to do this because scons overrides PYTHONPATH and does not preserve
# what is provided by the OS.  The custom variable name won't be overwritten.
export PYMOX="${NACL_SDK_ROOT}/third_party/pymox"
exec /bin/bash $CURR_DIR/third_party/swtoolkit/hammer.sh $*
