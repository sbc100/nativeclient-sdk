#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-x264-snapshot-20091023-2245.sh
#
# usage:  nacl-x264-snapshot-20091023-2245.sh
#
# this script downloads, patches, and builds x264 for Native Client
#

readonly URL=http://build.chromium.org/mirror/nacl/x264-snapshot-20091023-2245.tar.bz2
Ereadonly URL=http://downloads.videolan.org/pub/videolan/x264/snapshots/x264-snapshot-20091023-2245.tar.bz2
readonly PATCH_FILE=x264-snapshot-20091023-2245/nacl-x264-snapshot-20091023-2245.patch
readonly PACKAGE_NAME=x264-snapshot-20091023-2245

source ../common.sh


CustomExtractStep() {
  Banner "Untaring ${PACKAGE_NAME}"
  ChangeDir ${NACL_PACKAGES_REPOSITORY}
  Remove ${PACKAGE_NAME}
  tar xfj ${NACL_PACKAGES_TARBALLS}/${PACKAGE_NAME}.tgz
}


CustomConfigureStep() {
  Banner "Configuring ${PACKAGE_NAME}"
  # export the nacl tools
  export CC=${NACLCC}
  export CXX=${NACLCXX}
  export AR=${NACLAR}
  export RANLIB=${NACLRANLIB}
  export PKG_CONFIG_PATH=${NACL_SDK_USR_LIB}/pkgconfig
  export PKG_CONFIG_LIBDIR=${NACL_SDK_USR_LIB}
  export PATH=${NACL_BIN_PATH}:${PATH};
  ChangeDir ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
  ./configure \
    --cross-prefix=nacl- \
    --disable-asm \
    --disable-pthread \
    --prefix=${NACL_SDK_USR} \
    --exec-prefix=${NACL_SDK_USR} \
    --libdir=${NACL_SDK_USR_LIB}
}


CustomPackageInstall() {
  DefaultPreInstallStep
  DefaultDownloadStep
  CustomExtractStep
  DefaultPatchStep
  CustomConfigureStep
  DefaultBuildStep
  DefaultInstallStep
  DefaultCleanUpStep
}


CustomPackageInstall
exit 0
