#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-lua-5.1.4.sh
#
# usage:  nacl-lua-5.1.4.sh
#
# this script downloads, patches, and builds lua for Native Client 
#

readonly URL=http://build.chromium.org/mirror/nacl/lua-5.1.4.tar.gz
#readonly URL=http://www.lua.org/ftp/lua-5.1.4.tar.gz
readonly PATCH_FILE=lua-5.1.4/nacl-lua-5.1.4.patch
readonly PACKAGE_NAME=lua-5.1.4

source ../common.sh


CustomBuildStep() {
  ChangeDir ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
  make "CC=${NACLCC}" "PLAT=generic" "INSTALL_TOP=${NACL_SDK_USR}" clean
  make "CC=${NACLCC}" "PLAT=generic" "INSTALL_TOP=${NACL_SDK_USR}"
  # TODO: side-by-side install
  make "CC=${NACLCC}" "PLAT=generic" "INSTALL_TOP=${NACL_SDK_USR}" install
}


CustomPackageInstall() {
  DefaultPreInstallStep
  DefaultDownloadStep
  DefaultExtractStep
  DefaultPatchStep
  CustomBuildStep
  DefaultCleanUpStep
}


CustomPackageInstall
exit 0
