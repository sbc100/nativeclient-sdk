#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-flac-1.2.1.sh
#
# usage:  nacl-flac-1.2.1.sh
#
# this script downloads, patches, and builds flac for Native Client 
#

readonly URL=http://build.chromium.org/mirror/nacl/flac-1.2.1.tar.gz
#readonly URL=http://sourceforge.net/projects/flac/files/flac-src/\
#flac-1.2.1-src/flac-1.2.1.tar.gz/download
readonly PATCH_FILE=flac-1.2.1/nacl-flac-1.2.1.patch
readonly PACKAGE_NAME=flac-1.2.1

source ../common.sh


CustomConfigureStep() {
  Banner "Configuring ${PACKAGE_NAME}"
  # export the nacl tools
  export CC=${NACLCC}
  export CXX=${NACLCXX}
  export AR=${NACLAR}
  export RANLIB=${NACLRANLIB}
  export PKG_CONFIG_PATH=${NACL_SDK_USR}/lib/pkgconfig
  export PKG_CONFIG_LIBDIR=${NACL_SDK_USR}/lib
  export PATH=${NACL_BIN_PATH}:${PATH};
  export LIBS="-lnosys"
  ChangeDir ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
  Remove ${PACKAGE_NAME}-build
  MakeDir ${PACKAGE_NAME}-build
  ChangeDir ${PACKAGE_NAME}-build
  ../configure \
    --host=nacl \
    --disable-shared \
    --prefix=${NACL_SDK_USR} \
    --exec-prefix=${NACL_SDK_USR} \
    --libdir=${NACL_SDK_USR}/lib \
    --oldincludedir=${NACL_SDK_USR}/include \
    --enable-sse \
    --disable-3dnow \
    --disable-altivec \
    --disable-thorough-tests \
    --disable-oggtest \
    --disable-xmms-plugin \
    --without-metaflac-test-files \
    --with-http=off \
    --with-html=off \
    --with-ftp=off \
    --with-x=no
}


CustomPostConfigureStep() {
  # add stub includes
  mkdir netinet
  touch netinet/in.h
  # add ntohl for Native Client
  echo "#define ntohl(x) ((((x)>>24)&0xFF)|(((x)>>8)&0xFF00)|\
(((x)<<8)&0xFF0000)|(((x)<<24)&0xFF000000))" >> config.h
  # satisfy random, srandom, utime
  echo "#define random() rand()" >> config.h
  echo "#define srandom(x) srand(x)" >> config.h
}


CustomInstallStep() {
  # assumes pwd has makefile
  make install-exec
  (cd include; make install)
}


CustomPackageInstall() {
  DefaultPreInstallStep
  DefaultDownloadStep
  DefaultExtractStep
  DefaultPatchStep
  CustomConfigureStep
  CustomPostConfigureStep
  DefaultBuildStep
  CustomInstallStep
  DefaultCleanUpStep
}


CustomPackageInstall
exit 0

