#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-lame-398-2.sh
#
# usage:  nacl-lame-398-2.sh
#
# this script downloads, patches, and builds lame for Native Client 
#

readonly URL=http://build.chromium.org/mirror/nacl/lame-398-2.tar.gz
#readonly URL=http://sourceforge.net/projects/lame/files/lame/3.98.2/lame-398-2.tar.gz/download
readonly PATCH_FILE=lame-398-2/nacl-lame-398-2.patch
readonly PACKAGE_NAME=lame-398-2

source ../common.sh


DefaultPackageInstall
exit 0
