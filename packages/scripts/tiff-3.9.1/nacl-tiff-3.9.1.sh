#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-tiff-3.9.1.sh
#
# usage:  nacl-tiff-3.9.1.sh
#
# this script downloads, patches, and builds tiff for Native Client
#

readonly URL=http://build.chromium.org/mirror/nacl/tiff-3.9.1.tar.gz
#readonly URL=http://www.imagemagick.org/download/delegates/tiff-3.9.1.tar.gz
readonly PATCH_FILE=tiff-3.9.1/nacl-tiff-3.9.1.patch
readonly PACKAGE_NAME=tiff-3.9.1

source ../common.sh

DefaultPackageInstall
exit 0

