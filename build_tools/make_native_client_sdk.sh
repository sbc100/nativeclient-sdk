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

if ((NSIS)) && ((CygWin)) && ! [ -d NSIS ] ; then
  7z -oNSIS x "`dirname \"$0\"`"/nsis-2.46-Unicode-setup.exe
  mkdir -p NSIS/Contrib/Graphics/{Checks,Header,Icons,Wizard}
  for dirname in Checks Header Icons Wizard ; do
    mv NSIS/\$_OUTDIR/$dirname/* NSIS/Contrib/Graphics/$dirname
  done
  rmdir NSIS/\$_OUTDIR/{Checks,Header,Icons,Wizard,}
  mkdir "NSIS/Docs/Modern UI/images"
  ln "NSIS/Docs/Modern UI 2/images"/* "NSIS/Docs/Modern UI/images"
  mv NSIS/\$PLUGINSDIR/modern-header.bmp NSIS/Contrib/Graphics/Header/nsis.bmp
  mv NSIS/\$PLUGINSDIR/modern-wizard.bmp NSIS/Contrib/Graphics/Wizard/nsis.bmp
  mv NSIS/\$PLUGINSDIR/*.dll NSIS/Plugins
  rmdir NSIS/\$PLUGINSDIR
  chmod a+x NSIS/{,Bin,Contrib/UIs}/*.exe
  mkdir -p "MkLink/nsis"
  cp -aiv "NSIS/Examples/Plugin/nsis"/* "Mklink/nsis"
  cp -aiv "MkLink/Release Unicode/MkLink.dll" "NSIS/Plugins"
fi

declare -A packages
. "${0/.sh/.conf}"
. "`dirname \"$0\"`"/make_installer.inc

mkdir -p "`dirname \"$0\"`"/packages{,.src,.unpacked} setup
rm -rf packages.unpacked/{nacl-sdk.tgz,naclsdk_win_x86.tgz}
CYGWIN_PREFIX="third_party\\cygwin\\"

parse_setup_ini
download_package_dependences bash 0
reqpackages=()
sectionin=()
allinstpackages=()
allinstalledpackages=()
fix_setup_inf_info
mv "`dirname \"$0\"`"/nacl-sdk.tgz packages
version["native_client_sdk"]="$SDK_VERSION"
install["native_client_sdk"]="nacl-sdk.tgz"
sdesc["native_client_sdk"]="\"Native Client SDK - examples and documentation\""
category["native_client_sdk"]="00000000"
category_contents["00000000"]="${category_contents[00000000]} native_client_sdk"
requires["native_client_sdk"]="native_client_toolchain"
version["native_client_toolchain"]="$SDK_VERSION"
install["native_client_toolchain"]="naclsdk_win_x86.tgz"
sdesc["native_client_toolchain"]="\"Native Client SDK - toolchain\""
category["native_client_toolchain"]="00000000"
category_contents["00000000"]="${category_contents[00000000]} native_client_toolchain"
rm setup/*.lst.gz
download_addon_packages 2
if ((include_all_packages)) ; then
  download_all_packages 1
else
  for pkgname in "${!sectionin[@]}" ; do
    sectionin["$pkgname"]=" 1${sectionin[$pkgname]}"
  done
  for pkgname in "${!seed[@]}" ; do
    seed["$pkgname"]=" 1${seed[$pkgname]}"
  done
fi
for pkgname in "${allinstalledpackages[@]}" ; do
   prefix["$pkgname"]="$CYGWIN_PREFIX"
done
prefix["native_client_sdk"]=""
prefix["native_client_toolchain"]=""
fill_required_packages
fill_filetype_info

(
  cat <<END
RequestExecutionLevel user
SetCompressor $compressor
SetCompressorDictSize 128
Name "Native Client SDK"
OutFile ../../nacl-sdk.exe

Var SVV_CmdLineParameters
Var SVV_SelChangeInProgress

!include "FileFunc.nsh"
END
  declare_nsis_variables
  cat <<END

InstallDir "c:\\native_client_sdk_${SDK_VERSION//./_}"

!include "MUI2.nsh"
!include "Sections.nsh"

!define MUI_HEADERIMAGE
!define MUI_WELCOMEFINISHPAGE_BITMAP "\${NSISDIR}\\Contrib\\Graphics\\Wizard\\win.bmp"

!define MUI_WELCOMEPAGE_TITLE "Welcome to the Native Client SDK $SDK_VERSION Setup Wizard"
!define MUI_WELCOMEPAGE_TEXT "This wizard will guide you through the installation of the Native Client SDK $SDK_VERSION.\$\\r\$\\n\$\\r\$\\nNative Client SDK includes GNU toolchain adopted for Native Client use and some examples. You need Google Chrome to test the examples, but otherwise the SDK is self-contained.\$\\r\$\\n\$\\r\$\\n\$_CLICK"

!define MUI_COMPONENTSPAGE_SMALLDESC

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

!define MUI_FINISHPAGE_LINK "Visit the Native Client site for the latest news, FAQs and support"
!define MUI_FINISHPAGE_LINK_LOCATION "http://code.google.com/p/nativeclient-sdk/wiki/GettingStarted"

;!define MUI_FINISHPAGE_RUN
;!define MUI_FINISHPAGE_RUN_TEXT "Compile and Run Native Client Demos"
;!define MUI_FINISHPAGE_RUN_FUNCTION CompileAndRunDemo
;!define MUI_FINISHPAGE_NOREBOOTSUPPORT

!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show release notes"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION ShowReleaseNotes

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "" sec_Preinstall
  SectionIn `seq -s ' ' $((${#packages[@]}+3))`
  Push \$R0
  CreateDirectory "\$INSTDIR\\${CYGWIN_PREFIX}etc"
  CreateDirectory "\$INSTDIR\\${CYGWIN_PREFIX}etc\\setup"
  FileOpen \$R0 \$INSTDIR\\postinstall.sh w
  FileWrite \$R0 'export PATH=/usr/local/bin:/usr/bin:/bin\$\nexport CYGWIN="\$\$CYGWIN nodosfilewarning"\$\n'
  FileClose \$R0
  FileOpen \$R0 \$INSTDIR\\etc\\setup\\installed.log w
  FileWrite \$R0 "INSTALLED.DB 2\$\n"
  FileClose \$R0
  Pop \$R0
SectionEnd
END
  generate_section_list
  cat <<END
Section "" sec_PostInstall
  SectionIn `seq -s ' ' $((${#packages[@]}+3))`
  Push \$R0
  Push \$R1
  FileOpen \$R0 \$INSTDIR\\postinstall.sh a
  FileSeek \$R0 0 END
  FileWrite \$R0 "/bin/sort /etc/setup/installed.log -o /etc/setup/installed.db\$\nrm /etc/setup/installed.log\$\n"
  FileClose \$R0
  SetOutPath \$INSTDIR
  nsExec::ExecToLog '"${CYGWIN_PREFIX}bin\\bash" -c ./postinstall.sh'
  Delete \$INSTDIR\\postinstall.sh
  Delete \$INSTDIR\\${CYGWIN_PREFIX}bin\\bash.exe
  Rename \$INSTDIR\\${CYGWIN_PREFIX}bin\\bash4.exe \$INSTDIR\\${CYGWIN_PREFIX}bin\\bash.exe
  Delete \$INSTDIR\\${CYGWIN_PREFIX}bin\\sh.exe
  Rename \$INSTDIR\\${CYGWIN_PREFIX}bin\\sh4.exe \$INSTDIR\\${CYGWIN_PREFIX}bin\\sh.exe
  FileOpen \$R0 \$INSTDIR\\${CYGWIN_PREFIX}CygWin.bat w
  StrCpy \$R1 \$INSTDIR 1
  FileWrite \$R0 "@echo off\$\r\$\n\$\r\$\n\$R1:$\r\$\nchdir \$INSTDIR\\${CYGWIN_PREFIX//\//\\}bin\$\r\$\nbash --login -i$\r\$\n"
  FileClose \$R0
  SetOutPath \$INSTDIR\examples
  CreateDirectory "\$SMPROGRAMS\\Native Client SDK"
  CreateShortCut "\$SMPROGRAMS\\Native Client SDK\\StartDemo.lnk" "\$INSTDIR\\${CYGWIN_PREFIX}bin\\python.exe" "run.py"
  Pop \$R1
  Pop \$R0
SectionEnd
END
  generate_init_function 2
  generate_onselchange_function
  cat <<END
;Function CompileAndRunDemo
;  ExecShell "open" "\$SMPROGRAMS\\Native Client SDK\\StartDemo.lnk"
;FunctionEnd
Function ShowReleaseNotes
  ExecShell "open" "http://code.google.com/p/nativeclient-sdk/wiki/GettingStarted"
FunctionEnd
END
  echo "NSIS configuration file is created successfully..." >&2
  exit 0
) | sed \
  -e s"|\"/oname=sdk\\\\nacl-sdk\\\\|\"/oname=toolchain\\\\win_x86\\\\|"g \
  -e s"|\\\$INSTDIR\\\\sdk\\\\nacl-sdk\\\\|\\\$INSTDIR\\\\toolchain\\\\win_x86\\\\|"g \
  -e s"|\"/oname=native_client_sdk_${SDK_VERSION//./_}\\\\|\"/oname=|"g \
  -e s"|\\\$INSTDIR\\\\native_client_sdk_${SDK_VERSION//./_}\\\\|\$INSTDIR\\\\|"g \
  > make_native_client_sdk.nsi
