#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-fontconfig-2.7.3.sh
#
# usage:  nacl-fontconfig-2.7.3.sh
#
# this script downloads, patches, and builds fontconfig for Native Client.
#

readonly URL=http://build.chromium.org/mirror/nacl/fontconfig-2.7.3.tar.gz
#readonly URL=http://cgit.freedesktop.org/fontconfig/snapshot/fontconfig-2.7.3.tar.gz
readonly PATCH_FILE=fontconfig-2.7.3/nacl-fontconfig-2.7.3.patch
readonly MAKEFILE_PATCH_FILE=fontconfig-2.7.3/nacl-fontconfig-2.7.3.Makefile.patch
readonly PACKAGE_NAME=fontconfig-2.7.3

source ../common.sh

# fontconfig with-arch to be set for cross compiling
export with_arch=i686


CustomPatchStep() {
  ############################################################################
  # Recreation of configure/Makefile.in/config.sub/etc.
  # Will ask for autotools.
  ############################################################################
  # ChangeDir ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
  # ./autogen.sh --without-subdirs
  # make distclean
  ############################################################################
  # apply a small patch to generated config.sub to add nacl host type
  DefaultPatchStep
  chmod a+x ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}/{pseudo-gcc,configure,ltmain.sh}
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
  export PATH=${NACL_BIN_PATH}:${NACL_SDK_USR}/bin:${PATH};
  ChangeDir ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
  cd ${PACKAGE_NAME}-build
  # We'll not build host anyway
  CC_FOR_BUILD=${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}/pseudo-gcc \
  ../configure \
    --host=nacl \
    --disable-shared \
    --prefix=${NACL_SDK_USR} \
    --exec-prefix=${NACL_SDK_USR} \
    --libdir=${NACL_SDK_USR_LIB} \
    --oldincludedir=${NACL_SDK_USR_INCLUDE} \
    --with-http=off \
    --with-html=off \
    --with-ftp=off \
    --with-x=no
}


CustomPatchMakefileStep() {
  # fontconfig wants to build executable tools.  These tools aren't needed
  # for Native Client.  This function will patch the generated Makefile
  # to remove them.  (Use fontconfig tools on your build machine instead.)
  Patch ${PACKAGE_NAME} ${MAKEFILE_PATCH_FILE}
  ChangeDir ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}/${PACKAGE_NAME}-build
}


CustomPackageInstall() {
  DefaultPreInstallStep
  DefaultDownloadStep
  DefaultExtractStep
  CustomPatchStep
  CustomConfigureStep
  CustomPatchMakefileStep
  DefaultBuildStep
  DefaultInstallStep
  DefaultCleanUpStep
}


CustomPackageInstall
exit 0
