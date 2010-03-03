#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-zlib-1.2.3.sh
#
# usage:  nacl-zlib-1.2.3.sh
#
# this script downloads, patches, and builds zlib for Native Client 
#

readonly URL=http://build.chromium.org/mirror/nacl/zlib-1.2.3.tar.gz
#readonly URL=http://www.zlib.net/zlib-1.2.3.tar.gz
readonly PATCH_FILE=
readonly PACKAGE_NAME=zlib-1.2.3

source ../common.sh


CustomConfigureStep() {
  Banner "Configuring ${PACKAGE_NAME}"
  ChangeDir ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
  # TODO: side-by-side install
  CC=${NACLCC} AR="${NACLAR} -r" RANLIB=${NACLRANLIB} ./configure\
     --prefix=${NACL_SDK_USR}
}


CustomPackageInstall() {
  DefaultPreInstallStep
  DefaultDownloadStep
  DefaultExtractStep
  # zlib doesn't need patching, so no patch step
  CustomConfigureStep
  DefaultBuildStep
  DefaultInstallStep
  DefaultCleanUpStep
}


CustomPackageInstall
exit 0
