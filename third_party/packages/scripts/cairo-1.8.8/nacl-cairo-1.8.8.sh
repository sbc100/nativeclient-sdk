#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-cairo-1.8.8.sh
#
# usage:  nacl-cairo-1.8.8.sh
#
# this script downloads, patches, and builds cairo for Native Client.
#

readonly URL=http://build.chromium.org/mirror/nacl/cairo-1.8.8.tar.gz
#readonly URL=http://cairographics.org/releases/cairo-1.8.8.tar.gz
readonly PATCH_FILE=cairo-1.8.8/nacl-cairo-1.8.8.patch
readonly PACKAGE_NAME=cairo-1.8.8

source ../common.sh


DefaultPackageInstall
exit 0
