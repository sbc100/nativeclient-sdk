#!/usr/bin/env python
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

""" This script will parse the results file produced by MSTest.

The script takes a single argument containing the path to the Results.trx
file to parse. It will log relevant test run information, and exit with code 0
if all tests passed, or code 1 if some test failed.
"""

import sys
import xml.etree.ElementTree

MSTEST_NAMESPACE = 'http://microsoft.com/schemas/VisualStudio/TeamTest/2010'

def main():
  if len(sys.argv) < 2:
    print 'Must provide path to the Results.trx file'
    return 1

  print "Processing results: %s" % sys.argv[1]

  # Parse the xml results file
  tree = xml.etree.ElementTree.parse(sys.argv[1])
  root = tree.getroot()
  results_node = root.find('{%s}Results' % MSTEST_NAMESPACE)
  results = results_node.findall('{%s}UnitTestResult' % MSTEST_NAMESPACE)
  test_run_name = root.attrib['name']

  exit_code = 0

  # Print the results, note any failures by setting exit_code to 1
  for result in results:
    fail_message = None
    if 'outcome' not in result.attrib:
      result.attrib['outcome'] = 'Error'

    if result.attrib['outcome'] not in ('Passed', 'Inconclusive'):
      exit_code = 1
      fail_element = result.find('{%s}Output/{%s}ErrorInfo/{%s}Message' % (
          MSTEST_NAMESPACE, MSTEST_NAMESPACE, MSTEST_NAMESPACE))
      if fail_element is not None:
        fail_message = fail_element.text

    print 'TEST: %-35s [%s] [%s]' % (result.attrib['testName'],
        result.attrib.get('outcome'), result.attrib['duration'])
    if fail_message:
      print 'Reason: %s' % fail_message
    elif exit_code:
      print 'No error message given'

  return exit_code

if __name__ == '__main__':
  sys.exit(main())
