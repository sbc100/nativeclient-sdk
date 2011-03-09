#!/bin/bash -e

# Copyright (c) 2009 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Script to install everything needed to build chromium (well, ideally, anyway)
# See http://code.google.com/p/chromium/wiki/LinuxBuildInstructions
# and http://code.google.com/p/chromium/wiki/LinuxBuild64Bit

usage() {
  echo "Usage: $0 [--options]"
  echo "Options:"
  echo "--[no-]syms: enable or disable installation of debugging symbols"
  echo "--[no-]gold: enable or disable installation of gold linker"
  echo "--[no-]lib32: enable or disable installation of 32 bit libraries"
  echo "Script will prompt interactively if options not given."
  exit 1
}

while test "$1" != ""
do
  case "$1" in
  --syms)     do_inst_syms=1;;
  --no-syms)  do_inst_syms=0;;
  --gold)     do_inst_gold=1;;
  --no-gold)  do_inst_gold=0;;
  --lib32)    do_inst_lib32=1;;
  --no-lib32) do_inst_lib32=0;;
  *) usage;;
  esac
  shift
done

install_gold() {
  # Gold is optional; it's a faster replacement for ld,
  # and makes life on 2GB machines much more pleasant.

  # First make sure root can access this directory, as that's tripped up some folks.
  if sudo touch xyz.$$
  then
    sudo rm xyz.$$
  else
    echo root cannot write to the current directory, not installing gold
    return
  fi

  BINUTILS=binutils-2.20
  BINUTILS_URL=http://ftp.gnu.org/gnu/binutils/$BINUTILS.tar.bz2
  BINUTILS_SHA1=747e7b4d94bce46587236dc5f428e5b412a590dc

  test -f $BINUTILS.tar.bz2 || wget $BINUTILS_URL
  if test "`sha1sum $BINUTILS.tar.bz2|cut -d' ' -f1`" != "$BINUTILS_SHA1"
  then
    echo Bad sha1sum for $BINUTILS.tar.bz2
    exit 1
  fi

  cat > binutils-fix.patch <<__EOF__
--- binutils-2.20/gold/output.cc.orig	2009-11-17 17:40:49.000000000 -0800
+++ binutils-2.20/gold/output.cc	2009-11-17 18:27:21.000000000 -0800
@@ -22,6 +22,10 @@
 
 #include "gold.h"
 
+#if !defined(__STDC_FORMAT_MACROS)
+#define __STDC_FORMAT_MACROS
+#endif
+
 #include <cstdlib>
 #include <cstring>
 #include <cerrno>
@@ -29,6 +33,7 @@
 #include <unistd.h>
 #include <sys/mman.h>
 #include <sys/stat.h>
+#include <inttypes.h>
 #include <algorithm>
 #include "libiberty.h"
 
@@ -3505,11 +3510,11 @@
 		  Output_section* os = (*p)->output_section();
 		  if (os == NULL)
 		    gold_error(_("dot moves backward in linker script "
-				 "from 0x%llx to 0x%llx"),
+				 "from 0x%"PRIx64" to 0x%"PRIx64),
 			       addr + (off - startoff), (*p)->address());
 		  else
 		    gold_error(_("address of section '%s' moves backward "
-				 "from 0x%llx to 0x%llx"),
+				 "from 0x%"PRIx64" to 0x%"PRIx64),
 			       os->name(), addr + (off - startoff),
 			       (*p)->address());
 		}
__EOF__

  tar -xjvf $BINUTILS.tar.bz2
  cd $BINUTILS
  patch -p1 < ../binutils-fix.patch
  ./configure --prefix=/usr/local/gold --enable-gold
  make -j3
  if sudo make install
  then
    # Still need to figure out graceful way of pointing gyp to use
    # /usr/local/gold/bin/ld without requiring him to set environment
    # variables.  That will go into bootstrap-linux.sh when it's ready.
    echo "Installing gold as /usr/bin/ld."
    echo "To uninstall, do 'cd /usr/bin; sudo rm ld; sudo mv ld.orig ld'"
    test -f /usr/bin/ld && test ! -f /usr/bin/ld.orig && \
        sudo mv /usr/bin/ld /usr/bin/ld.orig
    sudo strip /usr/local/gold/bin/ld
    sudo ln -fs /usr/local/gold/bin/ld /usr/bin/ld.gold
    sudo ln -fs /usr/bin/ld.gold /usr/bin/ld
  else
    echo "make install failed, not installing gold"
  fi
}

if ! egrep -q 'Ubuntu (8\.04|8\.10|9\.04|9\.10|karmic|lucid)' /etc/issue; then
  echo "Only Ubuntu 8.04, 8.10, 9.04, and 9.10 are currently supported" >&2
  exit 1
fi

if ! uname -m | egrep -q "i686|x86_64"; then
  echo "Only x86 architectures are currently supported" >&2
  exit
fi

if [ "x$(id -u)" != x0 ]; then
  echo "Running as non-root user."
  echo "You might have to enter your password one or more times for 'sudo'."
  echo
fi

# Packages need for development
dev_list="apache2 bison fakeroot flex g++ g++-multilib gperf libapache2-mod-php5
          libasound2-dev libbz2-dev libcairo2-dev libdbus-glib-1-dev
          libgconf2-dev libgl1-mesa-dev libglu1-mesa-dev libglib2.0-dev
          libgtk2.0-dev libjpeg62-dev libnspr4-dev libnss3-dev libpam0g-dev
          libsqlite3-dev libxslt1-dev libxss-dev lighttpd mesa-common-dev
          msttcorefonts patch perl php5-cgi pkg-config python python2.5-dev rpm
          subversion ttf-dejavu-core ttf-kochi-gothic ttf-kochi-mincho wdiff"

# Full list of required run-time libraries
lib_list="libatk1.0-0 libc6 libasound2 libcairo2 libdbus-glib-1-2 libexpat1
          libfontconfig1 libfreetype6 libglib2.0-0 libgtk2.0-0 libnspr4-0d
          libnss3-1d libpango1.0-0 libpcre3 libpixman-1-0 libpng12-0 libstdc++6
          libsqlite3-0 libx11-6 libxau6 libxcb1 libxcomposite1
          libxcursor1 libxdamage1 libxdmcp6 libxext6 libxfixes3 libxi6
          libxinerama1 libxrandr2 libxrender1 zlib1g"

# Debugging symbols for all of the run-time libraries
dbg_list="libatk1.0-dbg libc6-dbg libcairo2-dbg
          libfontconfig1-dbg libglib2.0-0-dbg libgtk2.0-0-dbg libnspr4-0d-dbg
          libnss3-1d-dbg libpango1.0-0-dbg libpcre3-dbg libpixman-1-0-dbg
          libx11-6-dbg libxau6-dbg libxcb1-dbg libxcomposite1-dbg
          libxcursor1-dbg libxdamage1-dbg libxdmcp6-dbg libxext6-dbg
          libxfixes3-dbg libxi6-dbg libxinerama1-dbg libxrandr2-dbg
          libxrender1-dbg zlib1g-dbg"

# Standard 32bit compatibility libraries
cmp_list="ia32-libs lib32asound2-dev lib32readline5-dev lib32stdc++6 lib32z1
          lib32z1-dev libc6-dev-i386 libc6-i386"

# Waits for the user to press 'Y' or 'N'. Either uppercase of lowercase is
# accepted. Returns 0 for 'Y' and 1 for 'N'. If an optional parameter has
# been provided to yes_no(), the function also accepts RETURN as a user input.
# The parameter specifies the exit code that should be returned in that case.
# The function will echo the user's selection followed by a newline character.
# Users can abort the function by pressing CTRL-C. This will call "exit 1".
yes_no() {
  local c
  while :; do
    c="$(trap 'stty echo -iuclc icanon 2>/dev/null' EXIT INT TERM QUIT
         stty -echo iuclc -icanon 2>/dev/null
         dd count=1 bs=1 2>/dev/null | od -An -tx1)"
    case "$c" in
      " 0a") if [ -n "$1" ]; then
               [ $1 -eq 0 ] && echo "Y" || echo "N"
               return $1
             fi
             ;;
      " 79") echo "Y"
             return 0
             ;;
      " 6e") echo "N"
             return 1
             ;;
      "")    echo "Aborted" >&2
             exit 1
             ;;
      *)     # The user pressed an unrecognized key. As we are not echoing
             # any incorrect user input, alert the user by ringing the bell.
             (tput bel) 2>/dev/null
             ;;
    esac
  done
}

if test "$do_inst_syms" = ""
then
  echo "This script installs all tools and libraries needed to build Chromium."
  echo ""
  echo "For most of the libraries, it can also install debugging symbols, which"
  echo "will allow you to debug code in the system libraries. Most developers"
  echo "won't need these symbols."
  echo -n "Do you want me to install them for you (y/N) "
  if yes_no 1; then
    do_inst_syms=1
  fi
fi
if test "$do_inst_syms" = "1"; then
  echo "Installing debugging symbols."
else
  echo "Skipping installation of debugging symbols."
  dbg_list=
fi

sudo apt-get update

# We initially run "apt-get" with the --reinstall option and parse its output.
# This way, we can find all the packages that need to be newly installed
# without accidentally promoting any packages from "auto" to "manual".
# We then re-run "apt-get" with just the list of missing packages.
echo "Finding missing packages..."
packages="${dev_list} ${lib_list} ${dbg_list}"
if [ "$(uname -m)" = "x86_64" ]; then
  packages+=" ${cmp_list}"
fi
# Intentially leaving $packages unquoted so it's more readable.
echo "Packages required: " $packages
echo
new_list_cmd="sudo apt-get install --reinstall $(echo $packages)"
if new_list="$(yes n | LANG=C $new_list_cmd)"
then
  # We probably never hit this following line.
  echo "No missing packages, and the packages are up-to-date."
elif [ $? -eq 1 ]
then
  # We expect apt-get to have exit status of 1.
  # This indicates that we canceled the install with "yes n|".
  new_list=$(echo "$new_list" |
    sed -e '1,/The following NEW packages will be installed:/d;s/^  //;t;d')
  new_list=$(echo "$new_list" | sed 's/ *$//')
  if [ -z "$new_list" ] ; then
    echo "No missing packages, and the packages are up-to-date."
  else
    echo "Installing missing packages: $new_list."
    sudo apt-get install ${new_list}
  fi
  echo
else
  # An apt-get exit status of 100 indicates that a real error has occurred.

  # I am intentionally leaving out the '"'s around new_list_cmd,
  # as this makes it easier to cut and paste the output
  echo "The following command failed: " ${new_list_cmd}
  echo
  echo "It produces the following output:"
  yes n | $new_list_cmd || true
  echo
  echo "You will have to install the above packages yourself."
  echo
  exit 100
fi

# Some operating systems already ship gold
# (on Debian, you can probably do "apt-get install binutils-gold" to get it),
# but though Ubuntu toyed with shipping it, they haven't yet.
# So just install from source if it isn't the default linker.

case `ld --version` in
*gold*2.2*) ;;
* )
  if test "$do_inst_gold" = ""
  then
    echo "Gold is a new linker that links Chrome 5x faster than ld."
    echo "Don't use it if you need to link other apps (e.g. valgrind, wine)"
    echo -n "REPLACE SYSTEM LINKER ld with gold and back up ld? (y/N) "
    if yes_no 1; then
      do_inst_gold=1
    fi
  fi
  if test "$do_inst_gold" = "1"
  then
    # If the system provides gold, just install it.
    if apt-cache show binutils-gold >/dev/null; then
      echo "Installing binutils-gold. Backing up ld as ld.single."
      sudo apt-get install binutils-gold
    else
      # FIXME: avoid installing as /usr/bin/ld
      echo "Building binutils. Backing up ld as ld.orig."
      install_gold || exit 99
    fi
  else
    echo "Not installing gold."
  fi
esac

# Install 32bit backwards compatibility support for 64bit systems
if [ "$(uname -m)" = "x86_64" ]; then
  if test "$do_inst_lib32" = ""
  then
    echo "Installing 32bit libraries not already provided by the system"
    echo
    echo "While we only need to install a relatively small number of library"
    echo "files, we temporarily need to download a lot of large *.deb packages"
    echo "that contain these files. We will create new *.deb packages that"
    echo "include just the 32bit libraries. These files will then be found on"
    echo "your system in places like /lib32, /usr/lib32, /usr/lib/debug/lib32,"
    echo "/usr/lib/debug/usr/lib32. If you ever need to uninstall these files,"
    echo "look for packages named *-ia32.deb."
    echo "Do you want me to download all packages needed to build new 32bit"
    echo -n "package files (Y/n) "
    if yes_no 0; then
      do_inst_lib32=1
    fi
  fi
  if test "$do_inst_lib32" != "1"
  then
    echo "Exiting without installing any 32bit libraries."
    exit 0
  fi
  tmp=/tmp/install-32bit.$$
  trap 'rm -rf "${tmp}"' EXIT INT TERM QUIT
  mkdir -p "${tmp}/apt/lists/partial" "${tmp}/cache" "${tmp}/partial"
  touch "${tmp}/status"

  [ -r /etc/apt/apt.conf ] && cp /etc/apt/apt.conf "${tmp}/apt/"
  cat >>"${tmp}/apt/apt.conf" <<EOF
        Apt::Architecture "i386";
        Dir::Cache "${tmp}/cache";
        Dir::Cache::Archives "${tmp}/";
        Dir::State::Lists "${tmp}/apt/lists/";
        Dir::State::status "${tmp}/status";
EOF

  # Download 32bit packages
  echo "Computing list of available 32bit packages..."
  apt-get -c="${tmp}/apt/apt.conf" update

  echo "Downloading available 32bit packages..."
  apt-get -c="${tmp}/apt/apt.conf" \
          --yes --download-only --force-yes --reinstall  install \
          ${lib_list} ${dbg_list}

  # Open packages, remove everything that is not a library, move the
  # library to a lib32 directory and package everything as a *.deb file.
  echo "Repackaging and installing 32bit packages for use on 64bit systems..."
  for i in ${lib_list} ${dbg_list}; do
    orig="$(echo "${tmp}/${i}"_*_i386.deb)"
    compat="$(echo "${orig}" |
              sed -e 's,\(_[^_/]*_\)i386\(.deb\),-ia32\1amd64\2,')"
    rm -rf "${tmp}/staging"
    msg="$(fakeroot -u sh -exc '
      # Unpack 32bit Debian archive
      umask 022
      mkdir -p "'"${tmp}"'/staging/dpkg/DEBIAN"
      cd "'"${tmp}"'/staging"
      ar x "'${orig}'"
      tar zCfx dpkg data.tar.gz
      tar zCfx dpkg/DEBIAN control.tar.gz

      # Rename package, change architecture, remove dependencies
      sed -i -e "s/\(Package:.*\)/\1-ia32/"       \
             -e "s/\(Architecture:\).*/\1 amd64/" \
             -e "s/\(Depends:\).*/\1 ia32-libs/"  \
             -e "/Recommends/d"                   \
             -e "/Conflicts/d"                    \
          dpkg/DEBIAN/control

      # Only keep files that live in "lib" directories
      sed -i -e "/\/lib64\//d" -e "/\/.?bin\//d" \
             -e "s,\([ /]lib\)/,\132/g,;t1;d;:1" \
             -e "s,^/usr/lib32/debug\(.*/lib32\),/usr/lib/debug\1," \
          dpkg/DEBIAN/md5sums

      # Re-run ldconfig after installation/removal
      { echo "#!/bin/sh"; echo "[ \"x\$1\" = xconfigure ]&&ldconfig||:"; } \
        >dpkg/DEBIAN/postinst
      { echo "#!/bin/sh"; echo "[ \"x\$1\" = xremove ]&&ldconfig||:"; } \
        >dpkg/DEBIAN/postrm
      chmod 755 dpkg/DEBIAN/postinst dpkg/DEBIAN/postrm

      # Remove any other control files
      find dpkg/DEBIAN -mindepth 1 "(" -name control -o -name md5sums -o \
                       -name postinst -o -name postrm ")" -o -print |
        xargs -r rm -rf

      # Remove any files/dirs that live outside of "lib" directories
      find dpkg -mindepth 1 "(" -name DEBIAN -o -name lib ")" -prune -o \
                -print | tac | xargs -r -n 1 sh -c \
                "rm \$0 2>/dev/null || rmdir \$0 2>/dev/null || : "
      find dpkg -name lib64 -o -name bin -o -name "?bin" |
        tac | xargs -r rm -rf

      # Rename lib to lib32, but keep debug symbols in /usr/lib/debug/usr/lib32
      # That is where gdb looks for them.
      find dpkg -type d -o -path "*/lib/*" -print |
        xargs -r -n 1 sh -c "
          i=\$(echo \"\${0}\" |
               sed -e s,/lib/,/lib32/,g \
               -e s,/usr/lib32/debug\\\\\(.*/lib32\\\\\),/usr/lib/debug\\\\1,);
          mkdir -p \"\${i%/*}\";
          mv \"\${0}\" \"\${i}\""

      # Prune any empty directories
      find dpkg -type d | tac | xargs -r -n 1 rmdir 2>/dev/null || :

      # Create our own Debian package
      cd ..
      dpkg --build staging/dpkg .' 2>&1)"
    compat="$(eval echo $(echo "${compat}" |
                          sed -e 's,_[^_/]*_amd64.deb,_*_amd64.deb,'))"
    [ -r "${compat}" ] || {
      echo "${msg}" >&2
      echo "Failed to build new Debian archive!" >&2
      exit 1
    }

    msg="$(sudo dpkg -i "${compat}" 2>&1)" && {
        echo "Installed ${compat##*/}"
      } || {
        # echo "${msg}" >&2
        echo "Skipped ${compat##*/}"
      }
  done

  # Add symbolic links for developing 32bit code
  echo "Adding missing symbolic links, enabling 32bit code development..."
  for i in $(find /lib32 /usr/lib32 -maxdepth 1 -name \*.so.\* |
             sed -e 's/[.]so[.][0-9].*/.so/' |
             sort -u); do
    [ "x${i##*/}" = "xld-linux.so" ] && continue
    [ -r "$i" ] && continue
    j="$(ls "$i."* | sed -e 's/.*[.]so[.]\([^.]*\)$/\1/;t;d' |
         sort -n | tail -n 1)"
    [ -r "$i.$j" ] || continue
    sudo ln -s "${i##*/}.$j" "$i"
  done
fi
