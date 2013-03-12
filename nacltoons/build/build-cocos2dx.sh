#!/bin/bash
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

SCRIPT_DIR="$(cd $(dirname $0) && pwd)"

OUT_ROOT=${SCRIPT_DIR}/../out
mkdir -p ${OUT_ROOT}
OUT_ROOT="$(cd ${OUT_ROOT} && pwd)"

OUT_DIR=${OUT_ROOT}/cocos2dx
COCOS2DX_ROOT="$(cd ${SCRIPT_DIR}/../../third_party/cocos2d-x && pwd)"

# Pick platform directory for compiler.
readonly OS_NAME=$(uname -s)
if [ $OS_NAME = "Darwin" ]; then
  readonly OS_SUBDIR_SHORT="mac"
  readonly OS_JOBS=4
elif [ $OS_NAME = "Linux" ]; then
  readonly OS_SUBDIR_SHORT="linux"
  readonly OS_JOBS=$(cat /proc/cpuinfo | grep processor | wc -l)
else
  readonly OS_SUBDIR_SHORT="win"
  readonly OS_JOBS=1
fi

if [ $# -eq 1 ]; then
  TARGET=$1
fi

cd ${COCOS2DX_ROOT}

# $1=Dir $2=Arch $3=Libc $4=Config
BuildTargetArchLibcConfig() {
  ARCH=$2
  if [ "${ARCH}" = "i686" ]; then
    local ARCH_DIR=x86_32
  else
    local ARCH_DIR=${ARCH}
  fi

  NACL_CROSS_PREFIX=${ARCH}-nacl

  if [ $ARCH = "arm" ]; then
    local TOOLCHAIN_DIR=${OS_SUBDIR_SHORT}_arm_newlib
  else
    local TOOLCHAIN_DIR=${OS_SUBDIR_SHORT}_x86_newlib
  fi

  NACL_TOOLCHAIN_ROOT=${NACL_SDK_ROOT}/toolchain/${TOOLCHAIN_DIR}
  NACL_BIN_PATH=${NACL_TOOLCHAIN_ROOT}/bin

  # export nacl tools for direct use in patches.
  if [ "$4" = "Debug" ]; then
    local DEBUG=1
  else
    local DEBUG=0
  fi
  export LIB_DIR=${OUT_DIR}/lib/$3_${ARCH_DIR}
  export OBJ_DIR=${OUT_DIR}/obj/${ARCH_DIR}
  export NACLCC=${NACL_BIN_PATH}/${NACL_CROSS_PREFIX}-gcc
  export NACLCXX=${NACL_BIN_PATH}/${NACL_CROSS_PREFIX}-g++
  export NACLAR=${NACL_BIN_PATH}/${NACL_CROSS_PREFIX}-ar

  local NACL_ARCH=${ARCH}
  export NACL_ARCH
  export DEBUG
  export NACL_CC=${NACLCC}
  export NACL_CXX=${NACLCXX}
  export NACL_AR=${NACLAR}
  export COCOS2DX_PATH=${COCOS2DX_ROOT}/cocos2dx
  if [ -n "$NACLPORTS_ROOT" ]; then
      export NACLPORTS_ROOT
  fi

  local CMD="make -j ${OS_JOBS} -C $1 ${TARGET}"
  echo ${CMD}
  ${CMD}
}


# $1=Dir $2=Arch
BuildTargetArch() {
  BuildTargetArchLibcConfig $1 $2 newlib Release
  BuildTargetArchLibcConfig $1 $2 newlib Debug
}


# $1=Dir
BuildTarget() {
  if [ -n "${NACL_ARCH}" ]; then
    BuildTargetArch $1 ${NACL_ARCH}
  else
    BuildTargetArch $1 i686
    BuildTargetArch $1 x86_64
    BuildTargetArch $1 arm
  fi
}


# $1=Dir
CopyHeaderDir() {
  mkdir -p ${OUT_DIR}/$1
  cp -u ${COCOS2DX_ROOT}/cocos2dx/$1/*.h ${OUT_DIR}/$1
}


CopyHeaders() {
  CopyHeaderDir .
  CopyHeaderDir actions
  CopyHeaderDir base_nodes
  CopyHeaderDir cocoa
  CopyHeaderDir draw_nodes
  CopyHeaderDir effects
  CopyHeaderDir include
  CopyHeaderDir kazmath/include/kazmath
  CopyHeaderDir kazmath/include/kazmath/GL
  CopyHeaderDir keypad_dispatcher
  CopyHeaderDir label_nodes
  CopyHeaderDir layers_scenes_transitions_nodes
  CopyHeaderDir menu_nodes
  CopyHeaderDir misc_nodes
  CopyHeaderDir particle_nodes
  CopyHeaderDir platform
  CopyHeaderDir platform/nacl
  CopyHeaderDir script_support
  CopyHeaderDir shaders
  CopyHeaderDir sprite_nodes
  CopyHeaderDir support
  CopyHeaderDir support/data_support
  CopyHeaderDir support/user_default
  CopyHeaderDir support/tinyxml2
  CopyHeaderDir text_input_node
  CopyHeaderDir textures
  CopyHeaderDir tilemap_parallax_nodes
  CopyHeaderDir touch_dispatcher
  cp -u ${COCOS2DX_ROOT}/CocosDenshion/include/*.h ${OUT_DIR}
  cp -u ${COCOS2DX_ROOT}/scripting/lua/cocos2dx_support/*.h ${OUT_DIR}
  cp -u ${COCOS2DX_ROOT}/scripting/lua/tolua/tolua++.h ${OUT_DIR}
}

echo '@@@BUILD_STEP build cocos2dx@@@'
BuildTarget cocos2dx/proj.nacl

echo '@@@BUILD_STEP build CocosDenshion@@@'
BuildTarget CocosDenshion/proj.nacl

echo '@@@BUILD_STEP build Box2D@@@'
BuildTarget external/Box2D/proj.nacl

echo '@@@BUILD_STEP build lua@@@'
BuildTarget scripting/lua/proj.nacl

echo 'Copying headers'
CopyHeaders
