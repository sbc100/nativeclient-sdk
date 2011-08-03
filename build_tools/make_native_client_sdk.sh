#!/bin/bash
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

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
      5. Ready to use file make_sdk_installer.nsi for NSIS installer
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
    wget http://commondatastorage.googleapis.com/nativeclient-mirror/nacl/cygwin_mirror/hermetic_cygwin_1_7_9-0_1.exe -O cygwin_mini_setup.exe
    chmod a+x cygwin_mini_setup.exe
    "`cygpath $COMSPEC`" /C start /WAIT ".\\cygwin_mini_setup" /CYGPORT /S "/D=`cygpath -w $PWD/hermetic_cygwin`"
  fi
  exec "`cygpath $COMSPEC`" /C "`cygpath -w $PWD/hermetic_cygwin/bin/bash`" "`cygpath -w $0`" "$@"
fi

if ((BASH_VERSINFO[0]<4)) ; then
  echo "You need Bash4 to use this script" >&2
  exit 1
fi

# Filetype info
declare -A filetype

# $1 is the package name.  Unpacks using tar into the packages.unpacked
# directory.  The unpacked files are put into packages.unpacked/<package_name>,
# where <package_name> is the same as the tarball name.  This is required for
# the generate_section_commands routine later on.
tar_extract_package() {
  local instname=$1
  local install_dir=packages.unpacked/`basename $instname`
  if ((verbose)) ; then
    echo "Unpacking "$instname" to "$install_dir"..." >&2
  fi
  if [ ! -e $install_dir ] ; then
    mkdir -p $install_dir
  fi
  if ! tar xSvpf packages/$instname -C$install_dir ; then
    # Tar should have complained for us
    exit $?
  fi
}

# Scan all the directories in package.unpacked and determine the type of each
# node: "file" or "directory".  Run this step prior to running the
# generate_section_commands step.
fill_filetype_info() {
  # Corner cases: files created by post-install scripts, mistakes, etc
  filetype=([bin]="directory"
            [usr/bin]="directory"
            [lib]="directory"
            [usr/lib]="directory"
            [lib/icu/current/Makefile.inc]="file"
            [lib/rpm/rpmv.exe]="file"
            [usr/sbin/sendmail]="file"
            [usr/share/man/man1/gcc.1.gz]="file"
            [usr/share/man/man1/g++.1.gz]="file"
            [usr/share/man/man1/g77.1.gz]="file"
            [usr/share/man/man1/mf.1]="file"
            [usr/share/terminfo/E/Eterm]="file"
            [usr/share/terminfo/N/NCR260VT300WPP0]="file")
  if ((verbose)) ; then echo "Scanning filetypes..." >&2 ; fi
  for name in `
    for package in packages.unpacked/* ; do (
      cd $package
      if ((verbose)) ; then
        echo "Find files in archive: \"${package#*/}\"..." >&2
      fi
      find -type f
    ) done
  ` ; do
    if [[ "${name:0:10}" = "./usr/bin/" ]] ; then
      filetype["bin/${name:10}"]="file"
    elif [[ "${name:0:10}" = "./usr/lib/" ]] ; then
      filetype["lib/${name:10}"]="file"
    else
      filetype["${name:2}"]="file"
    fi
  done
  for name in `
    for package in packages.unpacked/* ; do (
      cd $package
      if ((verbose)) ; then
        echo "Find directories in archive: \"${package#*/}\"..." >&2
      fi
      find -type d
    ) done
  ` ; do
    if [[ "${name:0:10}" = "./usr/bin/" ]] ; then
      if [[ "bin/${filetype[${name:10}]}" = "file" ]] ; then
        echo "bin/${filetype[${name:10}]} - file and directory... oops?" >&2
        exit 1
      fi
      filetype["bin/${name:10}"]="directory"
    elif [[ "${name:0:10}" = "./usr/lib/" ]] ; then
      if [[ "lib/${filetype[${name:10}]}" = "file" ]] ; then
        echo "lib/${filetype[${name:10}]} - file and directory... oops?" >&2
        exit 1
      fi
      filetype["lib/${name:10}"]="directory"
    elif ((${#name}>1)) ; then
      if [[ "${filetype[${name:2}]}" = "file" ]] ; then
        echo "${filetype[${name:2}]} - file and directory... oops?" >&2
        exit 1
      fi
      filetype["${name:2}"]="directory"
    fi
  done
}

# Write the NSIS Section commands for a given package.  The Section
# commands are written to stdout.
# Parameters:
#   $1 The NSIS Section name; e.g. "!Native Client SDK".  This is written as
#      the first parameter to the Section command.
#   $2 The NSIS Section ID: e.g. NativeClientSDK.  This is written as the
#      second parameter to the Section command.
#   $3 The package name, this must be a tarball.  The name of the tarball is
#      assumed to be the same as the unpacked directory name.
#   $4- The "SectionIn" command parameters (optional).  If these parameters
#      exists, they are added in order as the value of the "SectionIn"
#      parameter.  Use this, for example, to mark Sections as RO (read-only).
generate_section_commands() {
  local section_name=$1; shift;
  local section_id=$1; shift;
  local pkgname="$1"; shift;
  # Write out the Section command and optional SectionIn command.
  echo "Section \"$section_name\" $section_id"
  # Write out the SectionIn parameters on the same line.
  if [ $# -gt 0 ] ; then
    echo -n "  SectionIn"
    while [ "$1" != "" ] ; do
      echo -n " $1"
      shift
    done
    echo ""
  fi
  echo "  SetOutPath \$INSTDIR"
  if [[ "$pkgname" != "" ]] ; then
    local pkgcontent="`tar tSvpf packages/$pkgname\"\" --numeric-owner`"
    local attrs uidgid size date time filename
    local -A createddirs
    createddirs[0]="done"
    echo "$pkgcontent" | grep "^[dhl-]" |
    while read -r attrs uidgid size date time filename ; do
      if [[ "${attrs:0:1}" = "h" ]] ; then
        filename="${filename%/* link to *}/"
      elif [[ "${attrs:0:1}" = "l" ]] ; then
        filename="${filename%/* -> *}/"
      elif [[ "${attrs:0:1}" = "-" ]] ; then
        if [[ "$filename" = */* ]] ; then
          filename="${filename%/*}/"
        else
          filename=""
        fi
      fi
      # Process directories: these each need a CreateDirectory command.
      if [[ "$filename" != "" ]] ; then
        filename="${filename:0:$((${#filename}-1))}"
        if [[ "${createddirs[$filename]}" != "done" ]] ; then
          echo "  CreateDirectory \"\$INSTDIR\\${filename//\//\\}\""
          createddirs["$filename"]="done"
        fi
      fi
    done
    # Process plain files.  Transform '/' path separators the '\' and write a
    # File command for each one.
    echo "$pkgcontent" | grep "^-" |
    while read -r attrs uidgid size date time filename ; do
      local fname=$filename
      fname="${fname//\//\\}"
      fname="${fname//\$/\$\$}"
      filename="${filename//\//\\}"
      echo "  File \"/oname=$fname\" \"packages.unpacked\\$pkgname\\$filename\""
    done
    # Process hard links.
    echo "$pkgcontent" | grep "^h" |
    while read -r attrs uidgid size date time filename ; do
      local linkname="${filename/ link to */}"
      local linktargetname="${filename/* link to /}"
      if [[ "${linkname:0:8}" = "usr/bin/" ]] ; then
        linkname="bin/${linkname:8}"
      elif [[ "${linkname:0:8}" = "usr/lib/" ]] ; then
        linkname="lib/${linkname:8}"
      fi
      if [[ "${linktargetname:0:8}" = "usr/bin/" ]] ; then
        linktargetname="bin/${linktargetname:8}"
      elif [[ "${linktargetname:0:8}" = "usr/lib/" ]] ; then
        linktargetname="lib/${linktargetname:8}"
      fi
      linkname="${linkname//\//\\}"
      linkname="${linkname//\$/\$\$}"
      linktargetname="${linktargetname//\//\\}"
      linktargetname="${linktargetname//\$/\$\$}"
      echo "  MkLink::Hard \"\$INSTDIR\\$linkname\" \"\$INSTDIR\\$linktargetname\""
    done
    # Process soft links.  There is some special processing to handle links
    # to executable files - the link node does not need to have the '.exe'
    # extension.
    echo "$pkgcontent" | grep "^l" |
    while read -r attrs uidgid size date time filename ; do
      local linkname="${filename/ -> */}"
      local linktargetname="${filename/* -> /}"
      if [[ "${linktargetname:0:2}" = "./" ]] ; then
        linktargetname="${linktargetname:2}"
      elif [[ "${linkname:0:8}" = "usr/bin/" ]] ; then
        linkname="bin/${linkname:8}"
      elif [[ "${linkname:0:8}" = "usr/lib/" ]] ; then
        linkname="lib/${linkname:8}"
      fi
      if [[ "${linkname%/*}/$linktargetname" = *//* ]] ; then
        linktargetname="/${linkname%/*}/$linktargetname"
        while [[ "$linktargetname" != //* ]] ; do
          linktargetname="${linktargetname%/*//*}//../${linktargetname#*//}"
        done
        linktargetname="${linktargetname:2}"
      fi
      local linktargetfile="/${linkname%/*}/$linktargetname"
      while [[ "$linktargetfile" = */../* ]] ; do
        local linktargetprefix="${linktargetfile%%/../*}"
        local linktargetsuffix="${linktargetfile#*/../}"
        linktargetfile="${linktargetprefix%/*}/$linktargetsuffix"
      done
      if [[ "${linktargetfile:0:9}" = "/usr/bin/" ]] ; then
        linktargetfile="bin/${linktargetfile:9}"
      elif [[ "${linktargetfile:0:9}" = "/usr/lib/" ]] ; then
        linktargetfile="lib/${linktargetfile:9}"
      else
        linktargetfile="${linktargetfile:1}"
      fi
      linkname="${linkname//\//\\}"
      linkname="${linkname//\$/\$\$}"
      if [[ "${filetype[$linktargetfile]}" = "file" ]] ; then
        linktargetname="${linktargetname//\//\\}"
        linktargetname="${linktargetname//\$/\$\$}"
        if [[ "${linkname:$((${#linkname}-4))}" = ".exe" ]] ||
           [[ "${linktargetfile:$((${#linktargetfile}-4))}" != ".exe" ]] ; then
          echo "  MkLink::SoftF \"\$INSTDIR\\$linkname\" \"$linktargetname\""
        else
          echo "  MkLink::SoftF \"\$INSTDIR\\$linkname.exe\" \"$linktargetname\""
        fi
      elif [[ "${filetype[$linktargetfile.exe]}" = "file" ]] ; then
        linktargetname="${linktargetname//\//\\}"
        linktargetname="${linktargetname//\$/\$\$}"
        if [[ "${linkname:$((${#linkname}-4))}" = ".exe" ]] ; then
          echo "  MkLink::SoftF \"\$INSTDIR\\$linkname\" \"$linktargetname.exe\""
        else
          echo "  MkLink::SoftF \"\$INSTDIR\\$linkname.exe\" \"$linktargetname.exe\""
        fi
      elif [[ "${filetype[$linktargetfile]}" = "directory" ]] ; then
        linktargetname="${linktargetname//\//\\}"
        linktargetname="${linktargetname//\$/\$\$}"
        echo "  MkLink::SoftD \"\$INSTDIR\\$linkname\" \"$linktargetname\""
      elif [ -f packages.unpacked/*/"$linktargetfile" ] ; then
        linktargetname="${linktargetname//\//\\}"
        linktargetname="${linktargetname//\$/\$\$}"
        if [[ "${linkname:$((${#linkname}-4))}" = ".exe" ]] ||
           [[ "${linktargetfile:$((${#linktargetfile}-4))}" != ".exe" ]] ; then
          echo "  MkLink::SoftF \"\$INSTDIR\\$linkname\" \"$linktargetname\""
        else
          echo "  MkLink::SoftF \"\$INSTDIR\\$linkname.exe\" \"$linktargetname\""
        fi
      elif [ -d packages.unpacked/*/"$linktargetfile" ] ; then
        linktargetname="${linktargetname//\//\\}"
        linktargetname="${linktargetname//\$/\$\$}"
        echo "  MkLink::SoftD \"\$INSTDIR\\$linkname\" \"$linktargetname\""
      elif [[ "$linktargetname" = "../share/webcheck/webcheck.py" ]] ; then
        echo "  MkLink::SoftF \"\$INSTDIR\\$linkname\" \"..\\share\\webcheck\\webcheck.py\""
      else
        echo "Can not determine the type of link \"$linktargetname\"" >&2
        exit 1
      fi
    done
  fi
  echo "SectionEnd"
}

SDK_VERSIONED_NAME=native_client_sdk_${SDK_VERSION//./_}

# Construct the install name based on the version string.
SDK_INSTALL_NAME="C:\\$SDK_VERSIONED_NAME"
echo "InstallDir \"$SDK_INSTALL_NAME\"" > sdk_install_name.nsh

# Unpack the SDK packages and produce the NSIS Section command files from the
# package contents.
mkdir -p "`dirname \"$0\"`"/packages{,.src,.unpacked}
rm -rf packages.unpacked/{nacl-sdk.tgz}

tar_extract_package nacl-sdk.tgz
fill_filetype_info

rm -f temp_script.nsh
generate_section_commands "!Native Client SDK" \
                          "NativeClientSDK" \
                          nacl-sdk.tgz \
                          RO > temp_script.nsh
# Do some special processing on the sdk_section.nsh file: strip out the line
# that creates the top-level SDK directory from the installer, and strip off
# the native_client_sdk_<version> prefix from the File commands.
sed \
  -e "/CreateDirectory \"\\\$INSTDIR\\\\$SDK_VERSIONED_NAME\"/d" \
  -e s"|\"/oname=$SDK_VERSIONED_NAME\\\\|\"/oname=|"g \
  -e s"|\\\$INSTDIR\\\\$SDK_VERSIONED_NAME\\\\|\$INSTDIR\\\\|"g \
  temp_script.nsh > sdk_section.nsh
rm -f temp_script.nsh

echo "NSIS configuration file is created successfully..." >&2

# Run the NSIS compiler.
if ((NSIS)) ; then
  if [ -e NSIS/makensis.exe ] ; then
    NSIS/makensis.exe /V2 make_sdk_installer.nsi
  else
    makensis /V2 make_sdk_installer.nsi
  fi
fi
