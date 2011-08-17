# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
#
# This script is used to package up the necessary files to put the NaCl gallery
# in the Chrome Web Store
#
# usage: ./cws_package.sh [outputname]

IMAGES="NaCl*.png"
MANIFEST="manifest.json"
ZIP_NAME="nacl-gallery-app.zip"

if [ "$1" ] ; then
  ZIP_NAME=$1
fi

zip ${ZIP_NAME} -r ${IMAGES} ${MANIFEST}