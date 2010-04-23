#!/usr/bin/python
#
# Copyright 2010, The Native Client SDK Authors.  All Rights Reserved.
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.
#

"""A tiny web server.

This is intended to be used for testing, and
only run from within the
examples directory.
"""

import BaseHTTPServer
import logging
import os
import SimpleHTTPServer
import SocketServer
import sys
import urlparse

logging.getLogger().setLevel(logging.INFO)

# Using 'localhost' means that we only accept connections
# via the loop back interface.
SERVER_PORT = 5103
SERVER_HOST = ''

# We only run from the examples directory (the one that contains scons-out), so
# that not too much is exposed via this HTTP server.  Everything in the
# directory is served, so there should never be anything potentially sensitive
# in the serving directory, especially if the machine might be a
# multi-user machine and not all users are trusted.  We only serve via
# the loopback interface.

SAFE_DIR_COMPONENTS = ['examples']
SAFE_DIR_SUFFIX = apply(os.path.join, SAFE_DIR_COMPONENTS)

def SanityCheckDirectory():
  if os.getcwd().endswith(SAFE_DIR_SUFFIX):
    return
  logging.error('httpd.py should only be run from the %s', SAFE_DIR_SUFFIX)
  logging.error('directory for testing purposes.')
  logging.error('We are currently in %s', os.getcwd())
  sys.exit(1)


# An HTTP server that will quit when |is_running| is set to False.
class QuittableHTTPServer(BaseHTTPServer.HTTPServer):
  def serve_forever(self):
    self.is_running = True
    while self.is_running:
      self.handle_request()

  def shutdown(self):
    self.is_running = False
    return 1


# A small handler that looks for '?quit' query in the path and shuts itself
# down if it finds that parameter.
class QuittableHTTPHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
  def do_GET(self):
    (_, _, _, query, _) = urlparse.urlsplit(self.path)
    if 'quit' in query:
      self.send_response(200, 'OK')
      self.send_header('Content-type', 'text/html')
      self.send_header('Content-length', '0')
      self.end_headers()
      self.server.shutdown()
      return

    SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)


def Run(server_address,
        server_class=QuittableHTTPServer,
        handler_class=QuittableHTTPHandler):
  httpd = server_class(server_address, handler_class)
  httpd.serve_forever()


if __name__ == '__main__':
  SanityCheckDirectory()
  if len(sys.argv) > 1:
    Run((SERVER_HOST, int(sys.argv[1])))
  else:
    Run((SERVER_HOST, SERVER_PORT))
  sys.exit(0)
