#!/bin/bash
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-install-all.sh
#
# usage:  nacl-install-all.sh
#
# This script builds all packages listed below for Native Client.
# Packages with no dependencies should be listed first.
#

set -o nounset
set -o errexit

(cd scripts/zlib-1.2.3; ./nacl-zlib-1.2.3.sh)
(cd scripts/jpeg-6b; ./nacl-jpeg-6b.sh)
(cd scripts/libpng-1.2.40; ./nacl-libpng-1.2.40.sh)
(cd scripts/tiff-3.9.1; ./nacl-tiff-3.9.1.sh)
(cd scripts/libogg-1.1.4; ./nacl-libogg-1.1.4.sh)
(cd scripts/libvorbis-1.2.3; ./nacl-libvorbis-1.2.3.sh)
(cd scripts/lame-398-2; ./nacl-lame-398-2.sh)
(cd scripts/faad2-2.7; ./nacl-faad2-2.7.sh)
(cd scripts/faac-1.28; ./nacl-faac-1.28.sh)
(cd scripts/libtheora-1.1.1; ./nacl-libtheora-1.1.1.sh)
(cd scripts/flac-1.2.1; ./nacl-flac-1.2.1.sh)
(cd scripts/speex-1.2rc1; ./nacl-speex-1.2rc1.sh)
(cd scripts/x264-snapshot-20091023-2245; ./nacl-x264-snapshot-20091023-2245.sh)
(cd scripts/lua-5.1.4; ./nacl-lua-5.1.4.sh)
(cd scripts/tinyxml; ./nacl-tinyxml.sh)
(cd scripts/expat-2.0.1; ./nacl-expat-2.0.1.sh)
(cd scripts/pixman-0.16.2; ./nacl-pixman-0.16.2.sh)
(cd scripts/gsl-1.9; ./nacl-gsl-1.9.sh)
(cd scripts/freetype-2.1.10; ./nacl-freetype-2.1.10.sh)
(cd scripts/fontconfig-2.7.3; ./nacl-fontconfig-2.7.3.sh)
(cd scripts/agg-2.5; ./nacl-agg-2.5.sh)
(cd scripts/cairo-1.8.8; ./nacl-cairo-1.8.8.sh)
(cd scripts/ImageMagick-6.5.4-10; ./nacl-ImageMagick-6.5.4-10.sh)
(cd scripts/ffmpeg-0.5; ./nacl-ffmpeg-0.5.sh)
(cd scripts/Mesa-7.6; ./nacl-Mesa-7.6.sh)
(cd scripts/libmodplug-0.8.7; ./nacl-libmodplug-0.8.7.sh)
#(cd scripts/OpenSceneGraph-2.9.7; ./nacl-OpenSceneGraph-2.9.7.sh)

exit 0
