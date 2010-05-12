#!/bin/bash
# Copyright (c) 2010 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-OpenSceneGraph-2.9.7.sh
#
# usage:  nacl-OpenSceneGraph-2.9.7.sh
#
# this script downloads, patches, and builds OpenSceneGraph for Native Client
#

readonly URL=http://build.chromium.org/mirror/nacl/OpenSceneGraph-2.9.7.zip
#readonly URL=http://www.openscenegraph.org/downloads/developer_releases/OpenSceneGraph-2.9.7.zip
readonly PATCH_FILE=OpenSceneGraph-2.9.7/nacl-OpenSceneGraph-2.9.7.patch
readonly PACKAGE_NAME=OpenSceneGraph-2.9.7

source ../common.sh


CustomDownloadStep() {
  cd ${NACL_PACKAGES_TARBALLS}
  # if matching tarball already exists, don't download again
  if ! Check ; then
    Fetch ${URL} ${PACKAGE_NAME}.zip
    if ! Check ; then
       Banner "${PACKAGE_NAME} failed checksum!"
       exit -1
    fi
  fi
}

CustomExtractStep() {
  Banner "Unzipping ${PACKAGE_NAME}"
  ChangeDir ${NACL_PACKAGES_REPOSITORY}
  Remove ${PACKAGE_NAME}
  unzip ${NACL_PACKAGES_TARBALLS}/${PACKAGE_NAME}.zip
}

CustomConfigureStep() {
  Banner "Configuring ${PACKAGE_NAME}"
  # export the nacl tools
  export CC=${NACLCC}
  export CXX=${NACLCXX}
  export AR=${NACLAR}
  export RANLIB=${NACLRANLIB}
  export PKG_CONFIG_PATH=${NACL_SDK_USR_LIB}/pkgconfig
  export PKG_CONFIG_LIBDIR=${NACL_SDK_USR_LIB}
  export PATH=${NACL_BIN_PATH}:${PATH};
  export NACL_INCLUDE=${NACL_SDK_USR_INCLUDE}
  ChangeDir ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
}

CustomInstallStep() {
  # copy libs and headers manually
  ChangeDir ${NACL_SDK_USR_INCLUDE}
  Remove ${PACKAGE_NAME}
  MakeDir ${PACKAGE_NAME}
  MakeDir ${PACKAGE_NAME}/osg
  MakeDir ${PACKAGE_NAME}/osgUtil
  readonly THIS_PACKAGE_PATH=${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
  cp -R ${THIS_PACKAGE_PATH}/include/osg ${PACKAGE_NAME}/osg
  cp -R ${THIS_PACKAGE_PATH}/include/osgUtil ${PACKAGE_NAME}/osgUtil
  cp -R ${THIS_PACKAGE_PATH}/include/OpenThreads ${PACKAGE_NAME}/OpenThreads
  ChangeDir ${NACL_SDK_USR_LIB}
  cp ${THIS_PACKAGE_PATH}/src/osg/libosg.a .
  cp ${THIS_PACKAGE_PATH}/src/osgUtil/libosgUtil.a .
  cp ${THIS_PACKAGE_PATH}/src/OpenThreads/libOpenThreads.a .
}

CustomPackageInstall() {
   DefaultPreInstallStep
   CustomDownloadStep
   CustomExtractStep
   DefaultPatchStep
   CustomConfigureStep
   DefaultBuildStep
   CustomInstallStep
   DefaultCleanUpStep
}

CustomPackageInstall
exit 0
