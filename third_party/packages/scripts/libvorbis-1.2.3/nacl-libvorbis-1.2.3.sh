#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-libvorbis-1.2.3.sh
#
# usage:  nacl-libvorbis-1.2.3.sh
#
# this script downloads, patches, and builds libvorbis for Native Client 
#

readonly URL=http://build.chromium.org/mirror/nacl/libvorbis-1.2.3.tar.gz
#readonly URL=http://downloads.xiph.org/releases/vorbis/libvorbis-1.2.3.tar.gz
readonly PATCH_FILE=libvorbis-1.2.3/nacl-libvorbis-1.2.3.patch
readonly PACKAGE_NAME=libvorbis-1.2.3

source ../common.sh


DefaultPackageInstall
exit 0
