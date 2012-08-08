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

  # Parse the xml results file
  tree = xml.etree.ElementTree.parse(sys.argv[1])
  root = tree.getroot()
  results_node = root.find('{%s}Results' % MSTEST_NAMESPACE)
  results = results_node.findall('{%s}UnitTestResult' % MSTEST_NAMESPACE)
  test_run_name = root.attrib['name']

  exit_code = 0

  # Print the results, note any failures by setting exit_code to 1
  print test_run_name
  for result in results:
    fail_message = 'None.'
    if result.attrib['outcome'] != 'Passed':
      exit_code = 1
      fail_element = result.find('{%s}Output/{%s}ErrorInfo/{%s}Message' % (
          MSTEST_NAMESPACE, MSTEST_NAMESPACE, MSTEST_NAMESPACE))
      if fail_element is not None:
        fail_message = fail_element.text
    print 'Test: %s, Duration: %s, Outcome: %s, Reason: %s\n' % (
        result.attrib['testName'], result.attrib['duration'],
        result.attrib['outcome'], fail_message)

  return exit_code

if __name__ == '__main__':
  sys.exit(main())
