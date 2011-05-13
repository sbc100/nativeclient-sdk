#!/bin/bash
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

declare -A short_desc prefix install
# Contents of categories
declare -A category_contents
# Packagenames: in request order and in install order
declare -A sectionin seed
declare -a allinstalledpackages
# Filetype info
declare -A filetype
# Sanitized names cache
declare -A sanitized_name
# Default installation type - filled in download_addon_packages if there are options
declare definsttype=1

sanitize_var_name() {
  local varname="$1"
  varname="${varname//-/_}"
  varname="${varname//./_}"
  varname="${varname//+/X}"
  echo -n "$varname"
}

declare_nsis_variables() {
  local outlist
  local pkgname
  if ((verbose)) ; then echo "Declare NSIS variables..." >&2 ; fi
  for pkgname in "${allinstalledpackages[@]}" ; do
    sanitized_name["$pkgname"]="`sanitize_var_name \"$pkgname\"`"
    outlist="$outlist
Var PKV_${sanitized_name[$pkgname]}"
  done
  echo "$outlist" | sort
  local secname
  for secname in "${!category_contents[@]}" ; do
    echo "$secname"
  done | sort |
  while read -r secname ; do
    local pkgname
    for pkgname in ${category_contents["$secname"]} ; do
      echo "$pkgname"
    done | sort |
    while read -r pkgname ; do
      echo "Var sv_${secname}_${sanitized_name[$pkgname]}"
    done
  done |
  sort
}

# $1 is the package name.  Unpacks using tar into the packages.unpacked
# directory.  Once the package is unpacked, this fills in the file type array
# for use later when generating the section lists.
unpack_all_packages() {
  for secname in "${allinstalledpackages[@]}" ; do
    local instname=${install["$secname"]}
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
  done
}

fill_filetype_info() {
  local IFS="
"
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

generate_unpack_tarball_text() {
  # $1 - package to unpack
  echo "  SetOutPath \$INSTDIR"
  local pkgname="$1"
  if [[ "$pkgname" != "" ]] ; then
    pkgname="`basename ${install[$pkgname]/.tar.bz2*/.tar.bz2}`"
  fi
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
      if [[ "$filename" = "usr/share/terminfo/e/" ]] ; then
        filename="usr/share/terminfo/E/"
      fi
      if [[ "${filename:0:8}" = "usr/bin/" ]] ; then
        filename="bin/${filename:8}"
      elif [[ "${filename:0:8}" = "usr/lib/" ]] ; then
        filename="lib/${filename:8}"
      fi
      if [[ "$filename" != "" ]] ; then
        filename="${filename:0:$((${#filename}-1))}"
        if [[ "${createddirs[$filename]}" != "done" ]] ; then
          echo "  CreateDirectory \"\$INSTDIR\\${prefix[$1]}${filename//\//\\}\""
          createddirs["$filename"]="done"
        fi
      fi
    done
    if [[ "$1" = "libgcj-common" ]] ; then
      echo "  CreateDirectory \"\$INSTDIR\\${prefix[$1]}lib\\security\""
    fi
    echo "$pkgcontent" | grep "^-" |
    while read -r attrs uidgid size date time filename ; do
      if [[ "$filename" != "usr/share/WindowMaker/Icons/DefaultAppIcon.tiff" ]] &&
         [[ "$filename" != "usr/share/ri/1.8/system/Exception2MessageMapper/fail-i.yaml" ]] &&
         [[ "$filename" != "usr/share/ri/1.8/system/REXML/Parsers/XPathParser/predicate-i.yaml" ]] ; then
        local fname
        if [[ "${filename:0:8}" = "usr/bin/" ]] ; then
          fname="bin/${filename:8}"
        elif [[ "${filename:0:8}" = "usr/lib/" ]] ; then
          fname="lib/${filename:8}"
        elif [[ "$filename" = "usr/share/terminfo/e/Eterm" ]] ; then
          fname="usr/share/terminfo/E/Eterm"
        else
          fname="$filename"
        fi
        fname="${fname//\//\\}"
        fname="${fname//\$/\$\$}"
        filename="${filename//\//\\}"
        echo "  File \"/oname=${prefix[$1]}$fname\" \"packages.unpacked\\$pkgname\\$filename\""
      fi
    done
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
      echo "  MkLink::Hard \"\$INSTDIR\\${prefix[$1]}$linkname\" \"\$INSTDIR\\${prefix[$1]}$linktargetname\""
    done
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
          echo "  MkLink::SoftF \"\$INSTDIR\\${prefix[$1]}$linkname\" \"$linktargetname\""
        else
          echo "  MkLink::SoftF \"\$INSTDIR\\${prefix[$1]}$linkname.exe\" \"$linktargetname\""
        fi
      elif [[ "${filetype[$linktargetfile.exe]}" = "file" ]] ; then
        linktargetname="${linktargetname//\//\\}"
        linktargetname="${linktargetname//\$/\$\$}"
        if [[ "${linkname:$((${#linkname}-4))}" = ".exe" ]] ; then
          echo "  MkLink::SoftF \"\$INSTDIR\\${prefix[$1]}$linkname\" \"$linktargetname.exe\""
        else
          echo "  MkLink::SoftF \"\$INSTDIR\\${prefix[$1]}$linkname.exe\" \"$linktargetname.exe\""
        fi
      elif [[ "${filetype[$linktargetfile]}" = "directory" ]] ; then
        linktargetname="${linktargetname//\//\\}"
        linktargetname="${linktargetname//\$/\$\$}"
        echo "  MkLink::SoftD \"\$INSTDIR\\${prefix[$1]}$linkname\" \"$linktargetname\""
      elif [ -f packages.unpacked/*/"$linktargetfile" ] ; then
        linktargetname="${linktargetname//\//\\}"
        linktargetname="${linktargetname//\$/\$\$}"
        if [[ "${linkname:$((${#linkname}-4))}" = ".exe" ]] ||
           [[ "${linktargetfile:$((${#linktargetfile}-4))}" != ".exe" ]] ; then
          echo "  MkLink::SoftF \"\$INSTDIR\\${prefix[$1]}$linkname\" \"$linktargetname\""
        else
          echo "  MkLink::SoftF \"\$INSTDIR\\${prefix[$1]}$linkname.exe\" \"$linktargetname\""
        fi
      elif [ -d packages.unpacked/*/"$linktargetfile" ] ; then
        linktargetname="${linktargetname//\//\\}"
        linktargetname="${linktargetname//\$/\$\$}"
        echo "  MkLink::SoftD \"\$INSTDIR\\${prefix[$1]}$linkname\" \"$linktargetname\""
      elif [[ "$linktargetname" = "../share/webcheck/webcheck.py" ]] ; then
        echo "  MkLink::SoftF \"\$INSTDIR\\${prefix[$1]}$linkname\" \"..\\share\\webcheck\\webcheck.py\""
      else
        echo "Can not determine the type of link \"$linktargetname\"" >&2
        exit 1
      fi
    done
  fi
}

generate_section_list() {
  if ((verbose)) ; then echo "Declaring unpack sections..." >&2 ; fi
  local secname
  for secname in "${!category_contents[@]}" ; do
    echo "$secname"
  done | sort |
  while read -r secname ; do
    local pkgname groupprint=1
    for pkgname in ${category_contents["$secname"]} ; do
      echo "$pkgname"
    done | sort |
    { local groupprint=1
      while read -r pkgname ; do
        if ((groupprint)) ; then
          if [[ "$secname" != "_PostInstallLast" ]] &&
             [[ "$secname" != "_obsolete" ]] ; then
            echo "SectionGroup \"$secname\" sec_$secname"
            groupprint=0
          fi
        fi
        local insttar="${install[$pkgname]/.tar.bz2*/}"
        if [[ "$secname" = "_PostInstallLast" ]] ||
           [[ "$secname" = "_obsolete" ]] ; then
          echo -n "Section \"-${insttar/*\//}\" "
        elif [[ " ${seed[$pkgname]} " == *" $definsttype "* ]] ; then
          echo -n "Section \"!${insttar/*\//}\" "
        else
          echo -n "Section \"${insttar/*\//}\" "
        fi
        echo      "sec_${secname}_${sanitized_name[$pkgname]}"
        echo "  SectionIn${sectionin[$pkgname]}"
        echo "SectionEnd"
      done
      if ((!groupprint)) ; then
        echo "SectionGroupEnd"
      fi
    }
  done
  for secname in "${allinstalledpackages[@]}" ; do
    if ((verbose)) ; then echo "Package "$secname"..." >&2 ; fi
    echo "Section \"\" sec_PKG_${sanitized_name[$secname]}"
    echo "  SectionIn${sectionin[$secname]}"
    generate_unpack_tarball_text "$secname"
    echo "SectionEnd"
  done
  echo "!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN"
  for secname in "${!category_contents[@]}" ; do
    echo "$secname"
  done | sort |
  while read -r secname ; do
    local pkgname
    for pkgname in ${category_contents["$secname"]} ; do
      echo "$pkgname"
    done | sort |
    { local groupprint=1
      while read -r pkgname ; do
        if ((groupprint)) ; then
          if [[ "$secname" != "_PostInstallLast" ]] &&
             [[ "$secname" != "_obsolete" ]] ; then
            echo "  !insertmacro MUI_DESCRIPTION_TEXT \${sec_$secname} \"\""
            groupprint=0
          fi
        fi
        echo -n "  !insertmacro MUI_DESCRIPTION_TEXT \${"
        echo -n "sec_${secname}_${sanitized_name[$pkgname]}} "
        # Modern UI does not like long descriptions
        echo  "${short_desc[$pkgname]}"
      done
    }
  done
  echo "!insertmacro MUI_FUNCTION_DESCRIPTION_END"
}

generate_init_function() {
  # $1 - Section number for first options (others should follow)
  if ((verbose)) ; then echo "Declaring .onInit function..." >&2 ; fi
  echo "Function .onInit"
  local pkgname
  for pkgname in "${allinstalledpackages[@]}" ; do
    if [[ " ${seed[$pkgname]} " == *" $definsttype "* ]] ; then
      echo "  StrCpy \$PKV_${sanitized_name[$pkgname]} 9"
    elif [[ " ${sectionin[$pkgname]} " == *" $definsttype "* ]] ; then
      echo "  StrCpy \$PKV_${sanitized_name[$pkgname]} 1"
    else
      echo "  StrCpy \$PKV_${sanitized_name[$pkgname]} 0"
    fi
  done
  local secname
  for secname in "${!category_contents[@]}" ; do
    echo "$secname"
  done | sort |
  while read -r secname ; do
    local pkgname
    for pkgname in ${category_contents["$secname"]} ; do
      echo "$pkgname"
    done | sort |
    while read -r pkgname ; do
      if [[ " ${sectionin[$pkgname]} " == *" $definsttype "* ]] ; then
        echo "  StrCpy \$sv_${secname}_${sanitized_name[$pkgname]} 1"
      else
        echo "  StrCpy \$sv_${secname}_${sanitized_name[$pkgname]} 0"
      fi
    done
  done
  cat <<END
  \${GetParameters} \$SVV_CmdLineParameters
  Push \$R0
  ClearErrors
  \${GetOptions} \$cmdLineParams "/?" \$R0
  IfErrors +1 HelpMessage
  \${GetOptions} \$cmdLineParams "--help" \$R0
  IfErrors +3 +1
HelpMessage:
END
  echo -n "  MessageBox MB_OK \"Recognized common options:"
  echo -n "\$\\n  /D=InstDir - use InstDir as target instead of usual \$INSTDIR"
  echo -n "\$\\n  /NCRC - disables the CRC check"
  echo -n "\$\\n  /S - Silent install"
  echo "\""
  echo "  Abort"
  echo "  Pop \$R0"
  echo "FunctionEnd"
}

generate_onselchange_function(){
  if ((verbose)) ; then echo "Declaring .onSelChange function..." >&2 ; fi
  cat <<END
Function .onSelChange
  IntCmp \$SVV_SelChangeInProgress 0 +1 L1_SelChangeInProgress L1_SelChangeInProgress
  StrCpy \$SVV_SelChangeInProgress 1
  Push \$R0
  Push \$R1
END
  # sv_XXX/PKV_XXX variables contain information about previous state of affairs
  # Note: there are three states - 0 (not to be installed), 9 (to be installed)
  # and 1 (will be installed because some other package needs it).
  # First we check is some packages were deselected and remove all packages
  # which can not be installed now.
  local secname pkgname subpkgname
  for secname in "${!category_contents[@]}" ; do
    echo "$secname"
  done | sort |
  while read -r secname ; do
    local pkgname
    for pkgname in ${category_contents["$secname"]} ; do
      echo "$pkgname"
    done | sort |
    while read -r pkgname ; do
      if ((verbose)) ; then echo "Read state: \"$pkgname\"..." >&2 ; fi
      local PKV_name="PKV_${sanitized_name[$pkgname]}"
      local sc_name="${secname}_${sanitized_name[$pkgname]}"
      echo "  SectionGetFlags \${sec_$sc_name} \$R0"
      echo "  IntOp \$R1 \$R0 & \${SF_SELECTED}"
      echo "  IntCmp \$sv_$sc_name \$R1 L4_$sc_name"
      echo "  IntCmp \$sv_$sc_name 0 L1_$sc_name"
      echo "  StrCpy \$$PKV_name 0"
      echo "  Goto L4_$sc_name"
      echo "L1_$sc_name:"
      echo "  StrCpy \$$PKV_name 9"
      echo "L4_$sc_name:"
    done
  done
  # Now we mark all packages which can be installed or not installed as
  # not-installed
  for pkgname in "${allinstalledpackages[@]}" ; do
    echo "  IntCmp \$PKV_${sanitized_name[$pkgname]} 9 +2 +1 +2"
    echo "  StrCpy \$PKV_${sanitized_name[$pkgname]} 0"
  done
  # Put the dependences back
  for pkgname in "${allinstalledpackages[@]}" ; do
    if ((verbose)) ; then echo "Set state: \"$pkgname\"..." >&2 ; fi
    echo "  IntCmp \$PKV_${sanitized_name[$pkgname]} 9 +1 L5_${sanitized_name[$pkgname]} +1"
    for subpkgname in ${requires["$pkgname"]} ; do
      echo "  IntCmp \$PKV_${sanitized_name[$subpkgname]} 9 +2 +1 +2"
      echo "  StrCpy \$PKV_${sanitized_name[$subpkgname]} 1"
    done
    echo "L5_${sanitized_name[$pkgname]}:"
  done
  # Copy the calculated new state to the screen
  for pkgname in "${allinstalledpackages[@]}" ; do
    echo "  SectionSetFlags \${sec_PKG_${sanitized_name[$pkgname]}} \$PKV_${sanitized_name[$pkgname]}"
  done
  for secname in "${!category_contents[@]}" ; do
    echo "$secname"
  done | sort |
  while read -r secname ; do
    local pkgname
    for pkgname in ${category_contents["$secname"]} ; do
      echo "$pkgname"
    done | sort |
    while read -r pkgname ; do
      echo "  IntOp \$sv_${secname}_${sanitized_name[$pkgname]} \$PKV_${sanitized_name[$pkgname]} & 1"
      echo "  SectionSetFlags \${sec_${secname}_${sanitized_name[$pkgname]}} \$PKV_${sanitized_name[$pkgname]}"
    done
  done
  cat <<END
  Pop \$R1
  Pop \$R0
  StrCpy \$SVV_SelChangeInProgress 0
L1_SelChangeInProgress:
FunctionEnd
END
}
