/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**
 * @fileoverview This file provides 2 variables that can be used
 * with the BrowserChecker Javascript class.  This file can be read
 * in by examples.ezt/examples.html from code.google.com/chrome/nativeclient.
 * Users can create a BrowserChecker object using
 * |supported_versions.MIN_GALLERY_CHROME_VERSION| and
 * |supported_versions.MAX_GALLERY_CHROME_VERSION|.
 * This allows updating the versions directly on app-engine w/o changing
 * examples.ezt in perforce.
 */

// Create a namespace object
var supported_versions = supported_versions || {};
// Add the min/max values to the namespace
supported_versions.MIN_GALLERY_CHROME_VERSION = 10;
supported_versions.MAX_GALLERY_CHROME_VERSION = 11;
