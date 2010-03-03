#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-pixman-0.16.2.sh
#
# usage:  nacl-pixman-0.16.2.sh
#
# this script downloads, patches, and builds pixman for Native Client 
#

readonly URL=http://build.chromium.org/mirror/nacl/pixman-0.16.2.tar.gz
#readonly URL=http://cairographics.org/releases/pixman-0.16.2.tar.gz
readonly PATCH_FILE=pixman-0.16.2/nacl-pixman-0.16.2.patch
readonly PACKAGE_NAME=pixman-0.16.2

source ../common.sh

DefaultPackageInstall
exit 0
