#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-ImageMagick-6.5.4-10.sh
#
# usage:  nacl-ImageMagick-6.5.4-10.sh
#
# this script downloads, patches, and builds ImageMagick for Native Client
#

readonly URL=http://build.chromium.org/mirror/nacl/ImageMagick-6.5.4-10.tar.gz
#readonly URL=http://downloads.sourceforge.net/project/imagemagick/ImageMagick/6.5/ImageMagick-6.5.4-10.tar.gz
readonly PATCH_FILE=ImageMagick-6.5.4-10/nacl-ImageMagick-6.5.4-10.patch
readonly PACKAGE_NAME=ImageMagick-6.5.4-10

source ../common.sh


CustomBuildAndInstallStep() {
  # assumes pwd has makefile
  make clean
  make CFLAGS='-DSSIZE_MAX="((ssize_t)(~((size_t)0)>>1))"' \
    install-libLTLIBRARIES install-data-am -j4
}


CustomPackageInstall() {
  DefaultPreInstallStep
  DefaultDownloadStep
  DefaultExtractStep
  DefaultPatchStep
  DefaultConfigureStep
  CustomBuildAndInstallStep
  DefaultCleanUpStep
}

CustomPackageInstall
exit 0
