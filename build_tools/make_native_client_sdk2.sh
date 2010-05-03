#!/bin/bash
# Copyright 2010, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

declare NSIS=0 verbose=0

PATH=/usr/bin

SDK_VERION=0.0.0.0

while getopts "nvV:" flag ; do
  case "$flag" in
    n) NSIS=1 ;;
    V) SDK_VERSION="$OPTARG" ;;
    v) verbose=1 ;;
    ?) cat <<END
  Usage: $0 [-n] [-v]
  Flags:
    -n: run NSIS to create actual installer
    -v: be more verbose when processing data
    Place CygWin decription file (setup.ini) in current directory, run script.
    You'll get:
      1. Downloaded binary files in subdirectory "packages"
      2. Downloaded source files in subdirectory "packages.src"
      3. Unpacked files in subdirectory "packages.unpacked"
      4. Setup log files in subdirectory "setup"
      5. Ready to use file make_installer.nsi for NSIS installer
   setup.ini is here: http://mirrors.kernel.org/sourceware/cygwin/setup.ini
   It's not downloaded by script to keep it hermetic.

    NSIS file format is described here: http://nsis.sourceforge.net/Docs
END
      exit 1;;
  esac
done

declare CygWin=0
if [[ "`uname -s`" != "Darwin" ]] ; then
  if [[ "`uname -o`" = "Cygwin" ]] ; then
    CygWin=1
    declare need_restart=0
    if ((BASH_VERSINFO[0]<4)) ; then
      need_restart=1
    fi
    if ((NSIS)) && ! [ -x NSIS/makensis.exe ] && ! [ -x /bin/7z ] ; then
      need_restart=1
    fi
  fi
fi

# Can only happen on CygWin - we don't autoinstall tools on other platforms
if ((need_restart)) ; then
  if ! [ -x "$PWD/hermetic_cygwin/bin/7z" ] && ! [ -x "$PWD/hermetic_cygwin/bin/7z.exe" ] ; then
    wget http://build.chromium.org/mirror/nacl/cygwin_mirror/hermetic_cygwin_1_7_5-1_0.exe -O cygwin_mini_setup.exe
    chmod a+x cygwin_mini_setup.exe
    "`cygpath $COMSPEC`" /C start /WAIT ".\\cygwin_mini_setup" /CYGPORT /S "/D=`cygpath -w $PWD/hermetic_cygwin`"
  fi
  exec "`cygpath $COMSPEC`" /C "`cygpath -w $PWD/hermetic_cygwin/bin/bash`" "`cygpath -w $0`" "$@"
fi

if ((BASH_VERSINFO[0]<4)) ; then
  echo "You need Bash4 to use this script" >&2
  exit 1
fi

CYGWIN_PREFIX="third_party\\cygwin\\"

if ! patch --no-backup-if-mismatch <<END
--- make_native_client_sdk.nsi
+++ make_native_client_sdk.nsi
@@ -182,8 +182,7 @@
-SectionGroup "00000000" sec_00000000
-Section "!nacl-sdk.tgz" sec_00000000_native_client_sdk
+Section "!Examples & documentation" sec_00000000_native_client_sdk
   SectionIn 1 2
 SectionEnd
-Section "!naclsdk_win_x86.tgz" sec_00000000_native_client_toolchain
+Section "!Native Client Toolchain" sec_00000000_native_client_toolchain
   SectionIn 1 2
 SectionEnd
-SectionGroupEnd
+SectionGroup "CygWin" sec_00000000
@@ -427,2 +426,3 @@
 SectionEnd
+SectionGroupEnd
 Section "" sec_PKG_native_client_toolchain
@@ -431,2 +430,0 @@
-  CreateDirectory "\$INSTDIR\\sdk"
-  CreateDirectory "\$INSTDIR\\sdk\\nacl-sdk"
@@ -1742 +1739,0 @@
-  CreateDirectory "\$INSTDIR\\native_client_sdk_${SDK_VERSION//./_}"
@@ -1815 +1811,0 @@
-  File "/oname=examples\\hello_world\\make.cmd" "packages.unpacked\\nacl-sdk.tgz\\native_client_sdk_${SDK_VERSION//./_}\\examples\\hello_world\\make.cmd"
@@ -1823 +1818,0 @@
-  File "/oname=examples\\httpd.cmd" "packages.unpacked\\nacl-sdk.tgz\\native_client_sdk_${SDK_VERSION//./_}\\examples\\httpd.cmd"
@@ -1825 +1819,0 @@
-  File "/oname=examples\\make.cmd" "packages.unpacked\\nacl-sdk.tgz\\native_client_sdk_${SDK_VERSION//./_}\\examples\\make.cmd"
@@ -1828 +1821,0 @@
-  File "/oname=examples\\pi_generator\\make.cmd" "packages.unpacked\\nacl-sdk.tgz\\native_client_sdk_${SDK_VERSION//./_}\\examples\\pi_generator\\make.cmd"
@@ -1852 +1822,0 @@
-  File "/oname=examples\\tumbler\\make.cmd" "packages.unpacked\\nacl-sdk.tgz\\native_client_sdk_${SDK_VERSION//./_}\\examples\\tumbler\\make.cmd"
@@ -4054,3 +4046,5 @@
-  File "/oname=${CYGWIN_PREFIX}bin\\bash.exe" "packages.unpacked\\bash-4.1.5-0.tar.bz2\\usr\\bin\\bash.exe"
+  File "/oname=${CYGWIN_PREFIX}bin\\bash.exe" "packages.unpacked\\bash-3.2.49-23.tar.bz2\\usr\\bin\\bash.exe"
+  File "/oname=${CYGWIN_PREFIX}bin\\bash4.exe" "packages.unpacked\\bash-4.1.5-0.tar.bz2\\usr\\bin\\bash.exe"
   File "/oname=${CYGWIN_PREFIX}bin\\bashbug" "packages.unpacked\\bash-4.1.5-0.tar.bz2\\usr\\bin\\bashbug"
-  File "/oname=${CYGWIN_PREFIX}bin\\sh.exe" "packages.unpacked\\bash-4.1.5-0.tar.bz2\\usr\\bin\\sh.exe"
+  File "/oname=${CYGWIN_PREFIX}bin\\sh.exe" "packages.unpacked\\bash-3.2.49-23.tar.bz2\\usr\\bin\\sh.exe"
+  File "/oname=${CYGWIN_PREFIX}bin\\sh4.exe" "packages.unpacked\\bash-4.1.5-0.tar.bz2\\usr\\bin\\sh.exe"
@@ -4303,4 +4299,4 @@
+  MkLink::Hard "\$INSTDIR\\${CYGWIN_PREFIX}bin\\awk.exe" "\$INSTDIR\\${CYGWIN_PREFIX}bin\\gawk.exe"
   MkLink::Hard "\$INSTDIR\\${CYGWIN_PREFIX}bin\\gawk-3.1.7.exe" "\$INSTDIR\\${CYGWIN_PREFIX}bin\\gawk.exe"
   MkLink::Hard "\$INSTDIR\\${CYGWIN_PREFIX}bin\\pgawk-3.1.7.exe" "\$INSTDIR\\${CYGWIN_PREFIX}bin\\pgawk.exe"
   MkLink::Hard "\$INSTDIR\\${CYGWIN_PREFIX}usr\\share\\man\\man1\\gawk.1" "\$INSTDIR\\${CYGWIN_PREFIX}usr\\share\\man\\man1\\pgawk.1"
-  MkLink::SoftF "\$INSTDIR\\${CYGWIN_PREFIX}bin\\awk.exe" "gawk.exe"
@@ -7046,6 +7040,10 @@
 Section "" sec_PKG_make
   SectionIn 1 2
   SetOutPath \$INSTDIR
+  CreateDirectory "\$INSTDIR\\examples"
+  CreateDirectory "\$INSTDIR\\examples\\hello_world"
+  CreateDirectory "\$INSTDIR\\examples\\pi_generator"
+  CreateDirectory "\$INSTDIR\\examples\\tumbler"
   CreateDirectory "\$INSTDIR\\${CYGWIN_PREFIX}usr"
   CreateDirectory "\$INSTDIR\\${CYGWIN_PREFIX}bin"
   CreateDirectory "\$INSTDIR\\${CYGWIN_PREFIX}usr\\share"
@@ -7101,2 +7099,6 @@
   CreateDirectory "\$INSTDIR\\${CYGWIN_PREFIX}usr\\share\\man\\man1"
+  File "/oname=examples\\make.cmd" "packages.unpacked\\nacl-sdk.tgz\\native_client_sdk_${SDK_VERSION//./_}\\examples\\make.cmd"
+  File "/oname=examples\\hello_world\\make.cmd" "packages.unpacked\\nacl-sdk.tgz\\native_client_sdk_${SDK_VERSION//./_}\\examples\\hello_world\\make.cmd"
+  File "/oname=examples\\pi_generator\\make.cmd" "packages.unpacked\\nacl-sdk.tgz\\native_client_sdk_${SDK_VERSION//./_}\\examples\\pi_generator\\make.cmd"
+  File "/oname=examples\\tumbler\\make.cmd" "packages.unpacked\\nacl-sdk.tgz\\native_client_sdk_${SDK_VERSION//./_}\\examples\\tumbler\\make.cmd"
   File "/oname=${CYGWIN_PREFIX}bin\\make.exe" "packages.unpacked\\make-3.81-2.tar.bz2\\usr\\bin\\make.exe"
@@ -7433,6 +7435,7 @@
 Section "" sec_PKG_python
   SectionIn 1 2
   SetOutPath \$INSTDIR
+  CreateDirectory "\$INSTDIR\\examples"
   CreateDirectory "\$INSTDIR\\${CYGWIN_PREFIX}usr"
   CreateDirectory "\$INSTDIR\\${CYGWIN_PREFIX}bin"
   CreateDirectory "\$INSTDIR\\${CYGWIN_PREFIX}usr\\include"
@@ -7471,2 +7474,3 @@
   CreateDirectory "\$INSTDIR\\${CYGWIN_PREFIX}usr\\share\\man\\man1"
+  File "/oname=examples\\httpd.cmd" "packages.unpacked\\nacl-sdk.tgz\\native_client_sdk_${SDK_VERSION//./_}\\examples\\httpd.cmd"
   File "/oname=${CYGWIN_PREFIX}bin\\libpython2.5.dll" "packages.unpacked\\python-2.5.5-1.tar.bz2\\usr\\bin\\libpython2.5.dll"
@@ -8992,3 +8996,3 @@
-  MkLink::SoftF "\$INSTDIR\\${CYGWIN_PREFIX}bin\\python.exe" "python2.5.exe"
-  MkLink::SoftF "\$INSTDIR\\${CYGWIN_PREFIX}bin\\python-config" "python2.5-config"
-  MkLink::SoftF "\$INSTDIR\\${CYGWIN_PREFIX}lib\\libpython2.5.dll.a" "python2.5\\config\\libpython2.5.dll.a"
+  MkLink::Hard "\$INSTDIR\\${CYGWIN_PREFIX}bin\\python.exe" "\$INSTDIR\\${CYGWIN_PREFIX}bin\\python2.5.exe"
+  MkLink::Hard "\$INSTDIR\\${CYGWIN_PREFIX}bin\\python-config" "\$INSTDIR\\${CYGWIN_PREFIX}bin\\python2.5-config"
+  MkLink::Hard "\$INSTDIR\\${CYGWIN_PREFIX}lib\\python2.5.dll.a" "\$INSTDIR\\${CYGWIN_PREFIX}lib\\python2.5\\config\\libpython2.5.dll.a"
@@ -9032,3 +9035,3 @@
-  !insertmacro MUI_DESCRIPTION_TEXT \${sec_00000000} ""
   !insertmacro MUI_DESCRIPTION_TEXT \${sec_00000000_native_client_sdk} "Native Client SDK - examples and documentation"
   !insertmacro MUI_DESCRIPTION_TEXT \${sec_00000000_native_client_toolchain} "Native Client SDK - toolchain"
+  !insertmacro MUI_DESCRIPTION_TEXT \${sec_00000000} "Small POSIX-like environment. Disable if you have POSIX-like environment installed and know how to use it."
END
  then
    exit 1
fi
if ((NSIS)) ; then
  if [ -e NSIS/makensis.exe ] ; then
    NSIS/makensis.exe /V2 make_native_client_sdk.nsi
  else
    makensis /V2 make_native_client_sdk.nsi
  fi
fi
