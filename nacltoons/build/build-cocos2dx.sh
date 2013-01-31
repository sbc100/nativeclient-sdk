#!/bin/bash
# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e

SCRIPT_DIR="$(cd $(dirname $0) && pwd)"
OUT_DIR=${SCRIPT_DIR}/../out/cocos2dx
COCOS2DX_ROOT=${SCRIPT_DIR}/../../third_party/cocos2d-x

NACLPORTS_PREFIX=${SCRIPT_DIR}/../out/naclports
NACLPORTS_INCLUDE=${NACLPORTS_PREFIX}/include

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


cd ${COCOS2DX_ROOT}

# $1=Dir $2=Arch $3=Libc $4=Config
BuildTargetArchLibcConfig() {
  if [ "$2" = "i686" ]; then
    ARCH_DIR=x86_32
  else
    ARCH_DIR=$2
  fi

  export LIB_DIR_ROOT=${OUT_DIR}/lib/$3_${ARCH_DIR}
  export NACL_ARCH=$2

  if [ "$4" = "Debug" ]; then
    export DEBUG=1
  else
    export DEBUG=0
  fi

  NACL_ARCH=$2
  NACL_CROSS_PREFIX=${NACL_ARCH}-nacl

  if [ $NACL_ARCH = "arm" ]; then
    local TOOLCHAIN_DIR=${OS_SUBDIR_SHORT}_arm_newlib
  else
    local TOOLCHAIN_DIR=${OS_SUBDIR_SHORT}_x86_newlib
  fi

  NACL_TOOLCHAIN_ROOT=${NACL_SDK_ROOT}/toolchain/${TOOLCHAIN_DIR}
  NACL_BIN_PATH=${NACL_TOOLCHAIN_ROOT}/bin

  # export nacl tools for direct use in patches.
  export NACLCC=${NACL_BIN_PATH}/${NACL_CROSS_PREFIX}-gcc
  export NACLCXX=${NACL_BIN_PATH}/${NACL_CROSS_PREFIX}-g++
  export NACLAR=${NACL_BIN_PATH}/${NACL_CROSS_PREFIX}-ar

  export CC=${NACLCC}
  export CXX=${NACLCXX}
  export AR=${NACLAR}
  export COCOS2DX_PATH=${COCOS2DX_ROOT}/cocos2dx
  export NACLPORTS_INCLUDE

  mkdir -p ${LIB_DIR_ROOT}/Debug
  mkdir -p ${LIB_DIR_ROOT}/Release
  make -C $1 clean
  make -j ${OS_JOBS} -C $1
}

# $1=Dir $2=Arch
BuildTargetArch() {
  BuildTargetArchLibcConfig $1 $2 newlib Release
  BuildTargetArchLibcConfig $1 $2 newlib Debug
}

# $1=Dir
BuildTarget() {
  BuildTargetArch $1 i686
  BuildTargetArch $1 x86_64
  BuildTargetArch $1 arm
}

# $1=Dir
CopyHeaderDir() {
  mkdir -p ${OUT_DIR}/$1
  cp ${COCOS2DX_ROOT}/cocos2dx/$1/*.h ${OUT_DIR}/$1
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
  CopyHeaderDir text_input_node
  CopyHeaderDir textures
  CopyHeaderDir tilemap_parallax_nodes
  CopyHeaderDir touch_dispatcher
}

echo "Building cocos2dx..."
BuildTarget cocos2dx/proj.nacl

echo "Copying headers..."
CopyHeaders
