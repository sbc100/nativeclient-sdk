#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-clean-all.sh
#
# usage:  nacl-clean-all.sh
#
# This script removes all packages installed for Native Client.
# Use nacl-install-all.sh to re-install all packages.
#
# TODO: if files other than those made by nacl-install-all.sh are
# placed in ${NACL_SDK_USR}, this script will need to exclude those
# files from deletion.
#

# need to define these before pulling in common.sh
readonly PACKAGE_NAME=
readonly URL=

source scripts/common.sh

# remove all tarballs
Remove ${NACL_PACKAGES_TARBALLS}
# remove all downloaded, extracted, patched sources in the repository
Remove ${NACL_PACKAGES_REPOSITORY}
# remove all installed headers, libraries, man pages, etc. in sdk usr
Remove ${NACL_SDK_USR}
# re-populate with empty directories
DefaultPreInstallStep
# remove specs file that adds include & lib paths to nacl-gcc
Remove ${NACL_SDK_BASE}/lib/gcc/nacl/4.2.2/specs
# remove the installed.txt file that lists which packages are installed
Remove ${NACL_PACKAGES}/installed.txt
exit 0
