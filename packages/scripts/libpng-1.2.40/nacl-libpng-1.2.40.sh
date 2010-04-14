#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-libpng-1.2.40.sh
#
# usage:  nacl-libpng-1.2.40.sh
#
# this script downloads, patches, and builds libpng for Native Client 
#

readonly URL=http://build.chromium.org/mirror/nacl/libpng-1.2.40.tar.gz
#readonly URL=http://downloads.sourceforge.net/libpng/libpng-1.2.40.tar.gz
readonly PATCH_FILE=libpng-1.2.40/nacl-libpng-1.2.40.patch
readonly PACKAGE_NAME=libpng-1.2.40

source ../common.sh

export LIBS=-lnosys

DefaultPackageInstall
exit 0

