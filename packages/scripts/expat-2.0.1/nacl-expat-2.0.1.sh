#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-expat-2.0.1.sh
#
# usage:  nacl-expat-2.0.1.sh
#
# this script downloads, patches, and builds expat for Native Client 
#

readonly URL=http://build.chromium.org/mirror/nacl/expat-2.0.1.tar.gz
#readonly URL=http://sourceforge.net/projects/expat/files/expat/2.0.1/expat-2.0.1.tar.gz/download
readonly PATCH_FILE=expat-2.0.1/nacl-expat-2.0.1.patch
readonly PACKAGE_NAME=expat-2.0.1

source ../common.sh

export LDFLAGS=-lnosys

DefaultPackageInstall
exit 0
