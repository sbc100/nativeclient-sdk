#!/usr/bin/python
# Copyright (c) 2009 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

import hashlib
import string
import sys

# usage:
#   python sha1sum.py filename
#
# will hash the filename in binary mode, generating output
# in the same format as the "sha1sum" utility:
#
#  da39a3ee5e6b4b0d3255bfef95601890afd80709 *filename
#
# Use this output as input to sha1check.py
#

for filename in sys.argv[1:]:
  try:
    # open the file in binary mode & generate sha1 hash
    f = open(filename, "rb")
    h = hashlib.sha1()
    h.update(f.read())
    filehash = h.hexdigest()
    f.close()
    print filehash.lower() + " *" + filename
  except IOError:
    print "sha1sum.py unable to open file " + filename
    sys.exit(-1)
  except:
    print "sha1sum.py encountered an unexpected error"
    sys.exit(-1)

# all files hashed with success
sys.exit(0)
