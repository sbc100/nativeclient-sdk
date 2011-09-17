// Copyright (c) 2011 The Native Client Authors. All Rights Reserved.
// Use of thjis source code is governed by a BSD-style license that can be found
// in the LICENSE file.

/**
 * @fileoverview This file contains the JavaScript required for testing the
 * WebGTT application using the nacltest.js browser testing framework.
 *
 * @author ragad@google.com (Raga Gopalakrishnan)
 */

/**
 * This function is triggered when the page has finished loading, and creates a
 * new Tester() object, sets up the tests, and runs them after the NaCl module
 * has finished loading.
 */
function main() {
  var tester = new Tester();
  var naclModule1 = document.getElementById('webgtt_test');
  setupTests(tester, naclModule1);
  tester.waitFor(naclModule1);
  tester.run();
}

/**
 * This function sets up the tests that need to be run.
 *
 * @param {Tester} tester The Tester object wrapper for the tests
 * @param {Element} plugin The reference to the NaCl module DOM object
 */
function setupTests(tester, plugin) {
  // TEST 1: Test coloring on a complete graph on three vertices.
  tester.addAsyncTest('Complete Graph', function(status) {
    status.expectMessageSequence(plugin, ['0,1,2']);
    plugin.postMessage('::0,1,1,1,0,1,1,1,0::0::');
  });
  // TODO(ragad): Add more tests.
}
