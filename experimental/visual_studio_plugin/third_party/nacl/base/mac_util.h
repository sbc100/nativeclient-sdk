// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MAC_UTIL_H_
#define BASE_MAC_UTIL_H_

#include <Carbon/Carbon.h>
#include <string>
#include <vector>

class FilePath;

#ifdef __OBJC__
@class NSBundle;
@class NSWindow;
#else
class NSBundle;
class NSWindow;
#endif

// Adapted from NSPathUtilities.h and NSObjCRuntime.h.
#if __LP64__ || NS_BUILD_32_LIKE_64
typedef unsigned long NSSearchPathDirectory;
#else
typedef unsigned int NSSearchPathDirectory;
#endif

namespace mac_util {

// Full screen modes, in increasing order of priority.  More permissive modes
// take predecence.
enum FullScreenMode {
  kFullScreenModeHideAll = 0,
  kFullScreenModeHideDock = 1,
  kFullScreenModeAutoHideAll = 2,
  kNumFullScreenModes = 3,

  // kFullScreenModeNormal is not a valid FullScreenMode, but it is useful to
  // other classes, so we include it here.
  kFullScreenModeNormal = 10,
};

std::string PathFromFSRef(const FSRef& ref);
bool FSRefFromPath(const std::string& path, FSRef* ref);

// Returns true if the application is running from a bundle
bool AmIBundled();

// Returns true if this process is marked as a "Background only process".
bool IsBackgroundOnlyProcess();

// Returns the main bundle or the override, used for code that needs
// to fetch resources from bundles, but work within a unittest where we
// aren't a bundle.
NSBundle* MainAppBundle();
FilePath MainAppBundlePath();

// Set the bundle that MainAppBundle will return, overriding the default value
// (Restore the default by calling SetOverrideAppBundle(nil)).
void SetOverrideAppBundle(NSBundle* bundle);
void SetOverrideAppBundlePath(const FilePath& file_path);

// Returns the creator code associated with the CFBundleRef at bundle.
OSType CreatorCodeForCFBundleRef(CFBundleRef bundle);

// Returns the creator code associated with this application, by calling
// CreatorCodeForCFBundleRef for the application's main bundle.  If this
// information cannot be determined, returns kUnknownType ('????').  This
// does not respect the override app bundle because it's based on CFBundle
// instead of NSBundle, and because callers probably don't want the override
// app bundle's creator code anyway.
OSType CreatorCodeForApplication();

// Searches for directories for the given key in only the user domain.
// If found, fills result (which must always be non-NULL) with the
// first found directory and returns true.  Otherwise, returns false.
bool GetUserDirectory(NSSearchPathDirectory directory, FilePath* result);

// Returns the ~/Library directory.
FilePath GetUserLibraryPath();

// Returns an sRGB color space.  The return value is a static value; do not
// release it!
CGColorSpaceRef GetSRGBColorSpace();

// Returns the color space being used by the main display.  The return value
// is a static value; do not release it!
CGColorSpaceRef GetSystemColorSpace();

// Add a full screen request for the given |mode|.  Must be paired with a
// ReleaseFullScreen() call for the same |mode|.  This does not by itself create
// a fullscreen window; rather, it manages per-application state related to
// hiding the dock and menubar.  Must be called on the main thread.
void RequestFullScreen(FullScreenMode mode);

// Release a request for full screen mode.  Must be matched with a
// RequestFullScreen() call for the same |mode|.  As with RequestFullScreen(),
// this does not affect windows directly, but rather manages per-application
// state.  For example, if there are no other outstanding
// |kFullScreenModeAutoHideAll| requests, this will reshow the menu bar.  Must
// be called on main thread.
void ReleaseFullScreen(FullScreenMode mode);

// Convenience method to switch the current fullscreen mode.  This has the same
// net effect as a ReleaseFullScreen(from_mode) call followed immediately by a
// RequestFullScreen(to_mode).  Must be called on the main thread.
void SwitchFullScreenModes(FullScreenMode from_mode, FullScreenMode to_mode);

// Set the visibility of the cursor.
void SetCursorVisibility(bool visible);

// Should windows miniaturize on a double-click (on the title bar)?
bool ShouldWindowsMiniaturizeOnDoubleClick();

// Activates the process with the given PID.
void ActivateProcess(pid_t);

// Pulls a snapshot of the entire browser into png_representation.
void GrabWindowSnapshot(NSWindow* window,
                        std::vector<unsigned char>* png_representation);

// Takes a path to an (executable) binary and tries to provide the path to an
// application bundle containing it. It takes the outermost bundle that it can
// find (so for "/Foo/Bar.app/.../Baz.app/..." it produces "/Foo/Bar.app").
//   |exec_name| - path to the binary
//   returns - path to the application bundle, or empty on error
FilePath GetAppBundlePath(const FilePath& exec_name);

// Set the Time Machine exclusion property for the given file.
bool SetFileBackupExclusion(const FilePath& file_path, bool exclude);

// Utility function to pull out a value from a dictionary, check its type, and
// return it.  Returns NULL if the key is not present or of the wrong type.
CFTypeRef GetValueFromDictionary(CFDictionaryRef dict,
                                 CFStringRef key,
                                 CFTypeID expected_type);

// Sets the process name as displayed in Activity Monitor to process_name.
void SetProcessName(CFStringRef process_name);

}  // namespace mac_util

#endif  // BASE_MAC_UTIL_H_
