#!/bin/bash
PATH=/bin
if [[ "$(uname -o 2>/dev/null)" != "Cygwin" ]] ; then
  echo "This script can only be used on Windows with CygWin !" >&2
  exit 1
fi
SRCDIR="$(dirname "$0")"
if [[ "$SRCDIR" = "." ]] ; then
  SRCDIR="$PWD"
fi
SDKROOTDIR="$SRCDIR/.."
if [ -f /proc/registry32/HKEY_LOCAL_MACHINE/Software/Microsoft/VisualStudio/10.0/Setup/VC/ProductDir ] ; then
  VCROOTDIR="$(cat /proc/registry32/HKEY_LOCAL_MACHINE/Software/Microsoft/VisualStudio/10.0/Setup/VC/ProductDir)"
elif [ -f /proc/registry32/HKEY_LOCAL_MACHINE/Software/Microsoft/VisualStudio/9.0/Setup/VC/ProductDir ] ; then
  VCROOTDIR="$(cat /proc/registry32/HKEY_LOCAL_MACHINE/Software/Microsoft/VisualStudio/9.0/Setup/VC/ProductDir)"
elif [ -f /proc/registry32/HKEY_LOCAL_MACHINE/Software/Microsoft/VisualStudio/8.0/Setup/VC/ProductDir ] ; then
  VCROOTDIR="$(cat /proc/registry32/HKEY_LOCAL_MACHINE/Software/Microsoft/VisualStudio/8.0/Setup/VC/ProductDir)"
elif [ -f /proc/registry32/HKEY_LOCAL_MACHINE/Software/Microsoft/VisualStudio/7.1/Setup/VC/ProductDir ] ; then
  VCROOTDIR="$(cat /proc/registry32/HKEY_LOCAL_MACHINE/Software/Microsoft/VisualStudio/7.1/Setup/VC/ProductDir)"
elif [ -f /proc/registry32/HKEY_LOCAL_MACHINE/Software/Microsoft/VisualStudio/7.0/Setup/VC/ProductDir ] ; then
  VCROOTDIR="$(cat /proc/registry32/HKEY_LOCAL_MACHINE/Software/Microsoft/VisualStudio/7.0/Setup/VC/ProductDir)"
else
  VCROOTDIR=""
  echo "Can not find Microsoft Visual Studio installation - change make.cmd files manually when it'll be installed" >&2
fi
#if [[ "$(uname -s)" = *WOW64* ]] ; then
#  if [ -f "$(cygpath "$VCROOTDIR")bin/x86_amd64/vcvarsx86_amd64.bat" ] ; then
#    VCSCRIPT="${VCROOTDIR}bin\\x86_amd64\\vcvarsx86_amd64.bat"
#  elif [ -f "$(cygpath "$VCROOTDIR")bin/amd64/vcvarsamd64.bat" ] ; then
#    VCSCRIPT="${VCROOTDIR}bin\\amd64\\vcvarsamd64.bat"
#  elif [ -f "$(cygpath "$VCROOTDIR")bin/vcvarsx86_amd64.bat" ] ; then
#    VCSCRIPT="${VCROOTDIR}bin\\vcvarsx86_amd64.bat"
#  elif [ -f "$(cygpath "$VCROOTDIR")bin/vcvarsamd64.bat" ] ; then
#    VCSCRIPT="${VCROOTDIR}bin\\vcvarsamd64.bat"
#  elif [ -f "$(cygpath "$VCROOTDIR")bin/vcvars64.bat" ] ; then
#    VCSCRIPT="${VCROOTDIR}bin\\vcvars64.bat"
#  # Test and use vcvarsall.bat last since it may be present but broken
#  elif [ -f "$(cygpath "$VCROOTDIR")vcvarsall.bat" ] ; then
#    VCSCRIPT="${VCROOTDIR}vcvarsall.bat\" \"x86_amd64"
#  else
#    echo "Can not find vcvarsall.bat. Native compilation is not supported. Please make sure you have Microsoft Visual Studio installed with \"X64 Compilers and Tools\" option selected." >&2
#    VCSCRIT="${VCROOTDIR}vcvarsall.bat\" \"x86_amd64"
#  fi
#else
  if [ -f "$(cygpath "$VCROOTDIR")bin/vcvars32.bat" ] ; then
    VCSCRIPT="${VCROOTDIR}bin\\vcvars32.bat"
  elif [ -f "$(cygpath "$VCROOTDIR")vcvars32.bat" ] ; then
    VCSCRIPT="${VCROOTDIR}vcvars32.bat"
  # Test and use vcvarsall.bat last since it may be present but broken
  elif [ -f "$(cygpath "$VCROOTDIR")vcvarsall.bat" ] ; then
    VCSCRIPT="${VCROOTDIR}vcvarsall.bat\" \"x86"
  else
    echo "Can not find vcvarsall.bat. Native compilation is not supported. Please make sure you have Microsoft Visual Studio installed." >&2
    VCSCRIT="${VCROOTDIR}vcvarsall.bat\" \"x86"
  fi
#fi
# Please keep this list and list in make_native_client_sdk.sh synchronyzed
echo -n $'@echo off\r\n\r\nset TMP_PATH=%PATH%\r\nset TMP_INCLUDE=%INCLUDE%\r\nset TMP_LIB=%LIB%\r\nset TMP_LIBPATH=%LIBPATH%\r\n\r\ncall "'"$VCSCRIPT"$'"\r\n\r\nREM Relative path of CygWin\r\nset CYGWIN=%~dp0%..\\third_party\\cygwin\\bin\r\n\r\nPATH=%CYGWIN%;%PATH%\r\n\r\nmake.exe %*\r\n\r\nset PATH=%TMP_PATH%\r\nset LIB=%TMP_LIB%\r\nset INCLUDE=%TMP_INCLUDE%\r\nset LIBPATH=%TMP_LIBPATH%\r\n' > "$SDKROOTDIR/debug_libs/make.cmd"
echo -n $'@echo off\r\n\r\nset TMP_PATH=%PATH%\r\nset TMP_INCLUDE=%INCLUDE%\r\nset TMP_LIB=%LIB%\r\nset TMP_LIBPATH=%LIBPATH%\r\n\r\ncall "'"$VCSCRIPT"$'"\r\n\r\nREM Relative path of CygWin\r\nset CYGWIN=%~dp0%..\\..\\third_party\\cygwin\\bin\r\n\r\nPATH=%CYGWIN%;%PATH%\r\n\r\nmake.exe %*\r\n\r\nPATH=%TMP_PATH%\r\nset LIB=%TMP_LIB%\r\nset INCLUDE=%TMP_INCLUDE%\r\nset LIBPATH=%TMP_LIBPATH%\r\n' > "$SDKROOTDIR/debug_libs/trusted_gpu/make.cmd"
