# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

set -o nounset
set -o errexit

# scripts that source this file must be run from within packages tree
readonly SAVE_PWD=$(pwd)

readonly START_DIR=$(cd $(dirname 0) ; pwd)
readonly NACL_PACKAGES=`expr ${START_DIR} : '\(.*\/packages\).*'`
readonly NACL_NATIVE_CLIENT_SDK=$(cd $NACL_PACKAGES/.. ; pwd)

# Pick platform directory for compiler.
readonly OS_NAME=$(uname -s)
if [ $OS_NAME = "Darwin" ]; then
  readonly OS_SUBDIR="mac"
  readonly OS_SUBDIR_SHORT="mac"
elif [ $OS_NAME = "Linux" ]; then
  readonly OS_SUBDIR="linux"
  readonly OS_SUBDIR_SHORT="linux"
else
  readonly OS_SUBDIR="windows"
  readonly OS_SUBDIR_SHORT="win"
fi

# locate default nacl_sdk toolchain
# TODO: x86 only at the moment
readonly NACL_TOP=$(cd $NACL_NATIVE_CLIENT_SDK/.. ; pwd)
readonly NACL_NATIVE_CLIENT=${NACL_TOP}/native_client
readonly NACL_SDK_BASE=${NACL_SDK_BASE:-\
${NACL_NATIVE_CLIENT_SDK}/toolchain/${OS_SUBDIR_SHORT}_x86}

# packages subdirectories
readonly NACL_PACKAGES_REPOSITORY=${NACL_PACKAGES}/repository
readonly NACL_PACKAGES_TARBALLS=${NACL_PACKAGES}/tarballs
readonly NACL_PACKAGES_SCRIPTS=${NACL_PACKAGES}/scripts

# sha1check python script
readonly SHA1CHECK=${NACL_PACKAGES_SCRIPTS}/sha1check.py

readonly NACL_BIN_PATH=${NACL_SDK_BASE}/bin
readonly NACLCC=${NACL_SDK_BASE}/bin/nacl-gcc
readonly NACLCXX=${NACL_SDK_BASE}/bin/nacl-g++
readonly NACLAR=${NACL_SDK_BASE}/bin/nacl-ar
readonly NACLRANLIB=${NACL_SDK_BASE}/bin/nacl-ranlib
readonly NACLLD=${NACL_SDK_BASE}/bin/nacl-ld

# NACL_SDK_GCC_SPECS_PATH is where nacl-gcc 'specs' file will be installed
readonly NACL_SDK_GCC_SPECS_PATH=${NACL_SDK_BASE}/lib/gcc/nacl64/4.4.3

# NACL_SDK_USR is where the headers, libraries, etc. will be installed
readonly NACL_SDK_USR=${NACL_SDK_BASE}/nacl/usr
readonly NACL_SDK_USR_INCLUDE=${NACL_SDK_USR}/include
readonly NACL_SDK_USR_LIB=${NACL_SDK_USR}/lib


######################################################################
# Helper functions
######################################################################

Banner() {
  echo "######################################################################"
  echo $*
  echo "######################################################################"
}


Usage() {
  egrep "^#@" $0 | cut --bytes=3-
}


ReadKey() {
  read
}


Fetch() {
  Banner "Fetching ${PACKAGE_NAME}"
  if which wget ; then
    wget $1 -O $2
  elif which curl ; then
    curl --location --url $1 -o $2
  else
     Banner "Problem encountered"
     echo "Please install curl or wget and rerun this script"
     echo "or manually download $1 to $2"
     echo
     echo "press any key when done"
     ReadKey
  fi

  if [ ! -s $2 ] ; then
    echo "ERROR: could not find $2"
    exit -1
  fi
}


Check() {
  # verify sha1 checksum for tarball
  local IN_FILE=${NACL_PACKAGES_SCRIPTS}/${PACKAGE_NAME}/${PACKAGE_NAME}.sha1
  if ${SHA1CHECK} <${IN_FILE} &>/dev/null; then
    return 0
  else
    return 1
  fi
}


Download() {
  cd ${NACL_PACKAGES_TARBALLS}
  # if matching tarball already exists, don't download again
  if ! Check ; then
    Fetch ${URL} ${PACKAGE_NAME}.tgz
    if ! Check ; then
       Banner "${PACKAGE_NAME} failed checksum!"
       exit -1
    fi
  fi
}


Patch() {
  local LOCAL_PACKAGE_NAME=$1
  local LOCAL_PATCH_FILE=$2
  if [ ${#LOCAL_PATCH_FILE} -ne 0 ]; then
    Banner "Patching ${LOCAL_PACKAGE_NAME}"
    cd ${NACL_PACKAGES_REPOSITORY}
    patch -p0 < ${NACL_PACKAGES_SCRIPTS}/${LOCAL_PATCH_FILE}
  fi
}


VerifyPath() {
  # make sure path isn't all slashes (possibly from an unset variable)
  local PATH=$1
  local TRIM=${PATH##/}
  if [ ${#TRIM} -ne 0 ]; then
    return 0
  else
    return 1
  fi
}


ChangeDir() {
  local NAME=$1
  if VerifyPath ${NAME}; then
    cd ${NAME}
  else
    echo "ChangeDir called with bad path."
    exit -1
  fi
}


Remove() {
  local NAME=$1
  if VerifyPath ${NAME}; then
    rm -rf ${NAME}
  else
    echo "Remove called with bad path."
    exit -1
  fi
}


MakeDir() {
  local NAME=$1
  if VerifyPath ${NAME}; then
    mkdir -p ${NAME}
  else
    echo "MakeDir called with bad path."
    exit -1
  fi
}


IsInstalled() {
  local LOCAL_PACKAGE_NAME=$1
  if [ ${#LOCAL_PACKAGE_NAME} -ne 0 ]; then
    if grep -qx ${LOCAL_PACKAGE_NAME} ${NACL_PACKAGES}/installed.txt &>/dev/null; then
      return 0
    else
      return 1
    fi
  else
    echo "IsInstalled called with possibly unset variable!"
  fi
}


AddToInstallFile() {
  local LOCAL_PACKAGE_NAME=$1
  if ! IsInstalled ${LOCAL_PACKAGE_NAME}; then
    echo ${LOCAL_PACKAGE_NAME} >> ${NACL_PACKAGES}/installed.txt
  fi
}


PatchSpecFile() {
  # fix up spaces so gcc sees entire path
  local SED_SAFE_SPACES_USR_INCLUDE=${NACL_SDK_USR_INCLUDE/ /\ /}
  local SED_SAFE_SPACES_USR_LIB=${NACL_SDK_USR_LIB/ /\ /}
  # have nacl-gcc dump specs file & add include & lib search paths
  ${NACL_SDK_BASE}/bin/nacl-gcc -dumpspecs |\
    sed "/*cpp:/{
      N
      s|$| -I${SED_SAFE_SPACES_USR_INCLUDE}|
    }" |\
    sed "/*link_libgcc:/{
      N
      s|$| -L${SED_SAFE_SPACES_USR_LIB}|
    }" >${NACL_SDK_GCC_SPECS_PATH}/specs
}


DefaultPreInstallStep() {
  cd ${NACL_TOP}
  MakeDir ${NACL_SDK_USR}
  MakeDir ${NACL_SDK_USR_INCLUDE}
  MakeDir ${NACL_SDK_USR_LIB}
  MakeDir ${NACL_PACKAGES_REPOSITORY}
  MakeDir ${NACL_PACKAGES_TARBALLS}
  Remove ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
}


DefaultDownloadStep() {
  Download
}


DefaultExtractStep() {
  Banner "Untaring ${PACKAGE_NAME}"
  ChangeDir ${NACL_PACKAGES_REPOSITORY}
  Remove ${PACKAGE_NAME}
  tar zxf ${NACL_PACKAGES_TARBALLS}/${PACKAGE_NAME}.tgz
}


DefaultPatchStep() {
  Patch ${PACKAGE_NAME} ${PATCH_FILE}
}


DefaultConfigureStep() {
  Banner "Configuring ${PACKAGE_NAME}"
  # export the nacl tools
  export CC=${NACLCC}
  export CXX=${NACLCXX}
  export AR=${NACLAR}
  export RANLIB=${NACLRANLIB}
  export PKG_CONFIG_PATH=${NACL_SDK_USR_LIB}/pkgconfig
  export PKG_CONFIG_LIBDIR=${NACL_SDK_USR_LIB}
  export PATH=${NACL_BIN_PATH}:${PATH};
  ChangeDir ${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}
  Remove ${PACKAGE_NAME}-build
  MakeDir ${PACKAGE_NAME}-build
  cd ${PACKAGE_NAME}-build
  ../configure \
    --host=nacl \
    --disable-shared \
    --prefix=${NACL_SDK_USR} \
    --exec-prefix=${NACL_SDK_USR} \
    --libdir=${NACL_SDK_USR_LIB} \
    --oldincludedir=${NACL_SDK_USR_INCLUDE} \
    --with-http=off \
    --with-html=off \
    --with-ftp=off \
    --with-x=no
}


DefaultBuildStep() {
  # assumes pwd has makefile
  make clean
  make -j4
}


DefaultInstallStep() {
  # assumes pwd has makefile
  make install
}


DefaultCleanUpStep() {
  PatchSpecFile
  AddToInstallFile ${PACKAGE_NAME}
  ChangeDir ${SAVE_PWD}
}


DefaultPackageInstall() {
  DefaultPreInstallStep
  DefaultDownloadStep
  DefaultExtractStep
  DefaultPatchStep
  DefaultConfigureStep
  DefaultBuildStep
  DefaultInstallStep
  DefaultCleanUpStep
}
