#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-agg-2.5.sh
#
# usage:  nacl-agg-2.5.sh
#
# this script downloads, patches, and builds agg for Native Client 
#

readonly URL=http://build.chromium.org/mirror/nacl/agg-2.5.tar.gz
#readonly URL=http://www.antigrain.com/agg-2.5.tar.gz
readonly PATCH_FILE=agg-2.5/nacl-agg-2.5.patch
readonly PACKAGE_NAME=agg-2.5

source ../common.sh


CustomBuildStep() {
  cd ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
  make NACL_BIN_PATH=${NACL_BIN_PATH}
}


CustomInstallStep() {
  # copy libs and headers manually
  ChangeDir ${NACL_SDK_USR_INCLUDE}
  Remove ${PACKAGE_NAME}
  MakeDir ${PACKAGE_NAME}
  readonly THIS_PACKAGE_PATH=${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
  cp -R ${THIS_PACKAGE_PATH}/include/*.h ${PACKAGE_NAME}/
  cp ${THIS_PACKAGE_PATH}/font_freetype/*.h ${PACKAGE_NAME}/
  ChangeDir ${NACL_SDK_USR_LIB}
  cp ${THIS_PACKAGE_PATH}/src/libagg.a .
  cp ${THIS_PACKAGE_PATH}/font_freetype/libaggfontfreetype.a .
}


CustomPackageInstall() {
  DefaultPreInstallStep
  DefaultDownloadStep
  DefaultExtractStep
  DefaultPatchStep
  CustomBuildStep
  CustomInstallStep
  DefaultCleanUpStep
}


CustomPackageInstall
exit 0
