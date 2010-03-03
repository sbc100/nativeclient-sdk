#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-clean-repository.sh
#
# usage:  nacl-clean-repository.sh
#
# This script remove all packages from the repository.
# Once packages are built and installed on Native Client, the respository
# can be removed if you no longer need the untarred sources.  This script
# does not remove the include headers or libs required for developement.
#

# need to define these before pulling in common.sh
readonly PACKAGE_NAME=
readonly URL=

source scripts/common.sh

# remove all downloaded, extracted, patched sources in the repository
Remove ${NACL_PACKAGES_REPOSITORY}
# re-populate with empty directories
DefaultPreInstallStep
exit 0
