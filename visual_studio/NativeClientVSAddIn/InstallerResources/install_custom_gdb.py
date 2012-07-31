#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Downloads a custom GDB and places it in the nacl_sdk folder.

The custom GDB is used in the add-in while we wait for nacl-gdb. The zip
file is downloaded to %TEMP% and gdb.exe is extracted to %NACL_SDK_ROOT%.
"""

import os
import urllib
import zipfile

OUTPUT_DIRECTORY = os.getenv('TEMP', None)
URL = ("http://www.chromium.org/nativeclient/how-tos/debugging-documentation"
       "/debugging-untrusted-code-with-a-special-gdb-build-on-windows"
       "/gdb-remote-x86-64.zip")


def main():
  nacl_sdk_root = os.getenv('NACL_SDK_ROOT', None)
  if nacl_sdk_root is None:
    raise Exception('Environment Variable NACL_SDK_ROOT is not set')
  if not os.path.exists(nacl_sdk_root):
    raise Exception('NaCl SDK does not exist at location: %s' % (nacl_sdk_root))
  zip_path = os.path.join(OUTPUT_DIRECTORY, "gdb-remote-x86-64.zip")

  print 'Downloading custom GDB from:'
  print URL
  urllib.urlretrieve(URL, zip_path)
  print 'Download complete. Unzipping...'
  zip_file = zipfile.ZipFile(zip_path)
  zip_file.extract('gdb-remote-x86-64/gdb.exe', nacl_sdk_root)
  zip_file.close()
  os.remove(zip_path)
  print 'Success!'

if __name__ == '__main__':
  main()
