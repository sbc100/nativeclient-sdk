// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/mac_util.h"

#import <Cocoa/Cocoa.h>

#include "base/file_path.h"
#include "base/logging.h"
#include "base/message_loop.h"
#include "base/scoped_cftyperef.h"
#include "base/sys_string_conversions.h"

namespace {

// a count of currently outstanding requests for full screen mode from browser
// windows, plugins, etc.
int g_full_screen_requests[mac_util::kNumFullScreenModes] = { 0, 0, 0};

// Sets the appropriate SystemUIMode based on the current full screen requests.
// Since only one SystemUIMode can be active at a given time, full screen
// requests are ordered by priority.  If there are no outstanding full screen
// requests, reverts to normal mode.  If the correct SystemUIMode is already
// set, does nothing.
void SetUIMode() {
  // Get the current UI mode.
  SystemUIMode current_mode;
  GetSystemUIMode(&current_mode, NULL);

  // Determine which mode should be active, based on which requests are
  // currently outstanding.  More permissive requests take precedence.  For
  // example, plugins request |kFullScreenModeAutoHideAll|, while browser
  // windows request |kFullScreenModeHideDock| when the fullscreen overlay is
  // down.  Precedence goes to plugins in this case, so AutoHideAll wins over
  // HideDock.
  SystemUIMode desired_mode = kUIModeNormal;
  SystemUIOptions desired_options = 0;
  if (g_full_screen_requests[mac_util::kFullScreenModeAutoHideAll] > 0) {
    desired_mode = kUIModeAllHidden;
    desired_options = kUIOptionAutoShowMenuBar;
  } else if (g_full_screen_requests[mac_util::kFullScreenModeHideDock] > 0) {
    desired_mode = kUIModeContentHidden;
  } else if (g_full_screen_requests[mac_util::kFullScreenModeHideAll] > 0) {
    desired_mode = kUIModeAllHidden;
  }

  if (current_mode != desired_mode)
    SetSystemUIMode(desired_mode, desired_options);
}

}  // end namespace

namespace mac_util {

std::string PathFromFSRef(const FSRef& ref) {
  scoped_cftyperef<CFURLRef> url(
      CFURLCreateFromFSRef(kCFAllocatorDefault, &ref));
  NSString *path_string = [(NSURL *)url.get() path];
  return [path_string fileSystemRepresentation];
}

bool FSRefFromPath(const std::string& path, FSRef* ref) {
  OSStatus status = FSPathMakeRef((const UInt8*)path.c_str(),
                                  ref, nil);
  return status == noErr;
}

// Adapted from http://developer.apple.com/carbon/tipsandtricks.html#AmIBundled
bool AmIBundled() {
  ProcessSerialNumber psn = {0, kCurrentProcess};

  FSRef fsref;
  if (GetProcessBundleLocation(&psn, &fsref) != noErr)
    return false;

  FSCatalogInfo info;
  if (FSGetCatalogInfo(&fsref, kFSCatInfoNodeFlags, &info,
                       NULL, NULL, NULL) != noErr) {
    return false;
  }

  return info.nodeFlags & kFSNodeIsDirectoryMask;
}

bool IsBackgroundOnlyProcess() {
  // This function really does want to examine NSBundle's idea of the main
  // bundle dictionary, and not the overriden MainAppBundle.  It needs to look
  // at the actual running .app's Info.plist to access its LSUIElement
  // property.
  NSDictionary* info_dictionary = [[NSBundle mainBundle] infoDictionary];
  return [[info_dictionary objectForKey:@"LSUIElement"] boolValue] != NO;
}

// No threading worries since NSBundle isn't thread safe.
static NSBundle* g_override_app_bundle = nil;

NSBundle* MainAppBundle() {
  if (g_override_app_bundle)
    return g_override_app_bundle;
  return [NSBundle mainBundle];
}

FilePath MainAppBundlePath() {
  NSBundle* bundle = MainAppBundle();
  return FilePath([[bundle bundlePath] fileSystemRepresentation]);
}

void SetOverrideAppBundle(NSBundle* bundle) {
  if (bundle != g_override_app_bundle) {
    [g_override_app_bundle release];
    g_override_app_bundle = [bundle retain];
  }
}

void SetOverrideAppBundlePath(const FilePath& file_path) {
  NSString* path = base::SysUTF8ToNSString(file_path.value());
  NSBundle* bundle = [NSBundle bundleWithPath:path];
  CHECK(bundle) << "Failed to load the bundle at " << file_path.value();

  SetOverrideAppBundle(bundle);
}

OSType CreatorCodeForCFBundleRef(CFBundleRef bundle) {
  OSType creator = kUnknownType;
  CFBundleGetPackageInfo(bundle, NULL, &creator);
  return creator;
}

OSType CreatorCodeForApplication() {
  CFBundleRef bundle = CFBundleGetMainBundle();
  if (!bundle)
    return kUnknownType;

  return CreatorCodeForCFBundleRef(bundle);
}

bool GetUserDirectory(NSSearchPathDirectory directory, FilePath* result) {
  DCHECK(result);
  NSArray* dirs =
      NSSearchPathForDirectoriesInDomains(directory, NSUserDomainMask, YES);
  if ([dirs count] < 1) {
    return false;
  }
  NSString* path = [dirs objectAtIndex:0];
  *result = FilePath([path fileSystemRepresentation]);
  return true;
}

FilePath GetUserLibraryPath() {
  FilePath user_library_path;
  if (!GetUserDirectory(NSLibraryDirectory, &user_library_path)) {
    LOG(WARNING) << "Could not get user library path";
  }
  return user_library_path;
}

CGColorSpaceRef GetSRGBColorSpace() {
  // Leaked.  That's OK, it's scoped to the lifetime of the application.
  static CGColorSpaceRef g_color_space_sRGB =
      CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
  LOG_IF(ERROR, !g_color_space_sRGB) << "Couldn't get the sRGB color space";
  return g_color_space_sRGB;
}

CGColorSpaceRef GetSystemColorSpace() {
  // Leaked.  That's OK, it's scoped to the lifetime of the application.
  // Try to get the main display's color space.
  static CGColorSpaceRef g_system_color_space =
      CGDisplayCopyColorSpace(CGMainDisplayID());

  if (!g_system_color_space) {
    // Use a generic RGB color space.  This is better than nothing.
    g_system_color_space = CGColorSpaceCreateDeviceRGB();

    if (g_system_color_space) {
      LOG(WARNING) <<
          "Couldn't get the main display's color space, using generic";
    } else {
      LOG(ERROR) << "Couldn't get any color space";
    }
  }

  return g_system_color_space;
}

// Add a request for full screen mode.  Must be called on the main thread.
void RequestFullScreen(FullScreenMode mode) {
  DCHECK_LT(mode, kNumFullScreenModes);
  if (mode >= kNumFullScreenModes)
    return;

  DCHECK_GE(g_full_screen_requests[mode], 0);
  g_full_screen_requests[mode] = std::max(g_full_screen_requests[mode] + 1, 1);
  SetUIMode();
}

// Release a request for full screen mode.  Must be called on the main thread.
void ReleaseFullScreen(FullScreenMode mode) {
  DCHECK_LT(mode, kNumFullScreenModes);
  if (mode >= kNumFullScreenModes)
    return;

  DCHECK_GT(g_full_screen_requests[mode], 0);
  g_full_screen_requests[mode] = std::max(g_full_screen_requests[mode] - 1, 0);
  SetUIMode();
}

// Switches full screen modes.  Releases a request for |from_mode| and adds a
// new request for |to_mode|.  Must be called on the main thread.
void SwitchFullScreenModes(FullScreenMode from_mode, FullScreenMode to_mode) {
  DCHECK_LT(from_mode, kNumFullScreenModes);
  DCHECK_LT(to_mode, kNumFullScreenModes);
  if (from_mode >= kNumFullScreenModes || to_mode >= kNumFullScreenModes)
    return;

  DCHECK_GT(g_full_screen_requests[from_mode], 0);
  DCHECK_GE(g_full_screen_requests[to_mode], 0);
  g_full_screen_requests[from_mode] =
      std::max(g_full_screen_requests[from_mode] - 1, 0);
  g_full_screen_requests[to_mode] =
      std::max(g_full_screen_requests[to_mode] + 1, 1);
  SetUIMode();
}

void SetCursorVisibility(bool visible) {
  if (visible)
    [NSCursor unhide];
  else
    [NSCursor hide];
}

bool ShouldWindowsMiniaturizeOnDoubleClick() {
  // We use an undocumented method in Cocoa; if it doesn't exist, default to
  // |true|. If it ever goes away, we can do (using an undocumented pref key):
  //   NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  //   return ![defaults objectForKey:@"AppleMiniaturizeOnDoubleClick"] ||
  //          [defaults boolForKey:@"AppleMiniaturizeOnDoubleClick"];
  BOOL methodImplemented =
      [NSWindow respondsToSelector:@selector(_shouldMiniaturizeOnDoubleClick)];
  DCHECK(methodImplemented);
  return !methodImplemented ||
      [NSWindow performSelector:@selector(_shouldMiniaturizeOnDoubleClick)];
}

void GrabWindowSnapshot(NSWindow* window,
    std::vector<unsigned char>* png_representation) {
  // Make sure to grab the "window frame" view so we get current tab +
  // tabstrip.
  NSView* view = [[window contentView] superview];
  NSBitmapImageRep* rep =
      [view bitmapImageRepForCachingDisplayInRect:[view bounds]];
  [view cacheDisplayInRect:[view bounds] toBitmapImageRep:rep];
  NSData* data = [rep representationUsingType:NSPNGFileType properties:nil];
  const unsigned char* buf = static_cast<const unsigned char*>([data bytes]);
  NSUInteger length = [data length];
  if (buf != NULL && length > 0){
    png_representation->assign(buf, buf + length);
    DCHECK(png_representation->size() > 0);
  }
}

void ActivateProcess(pid_t pid) {
  ProcessSerialNumber process;
  OSStatus status = GetProcessForPID(pid, &process);
  if (status == noErr) {
    SetFrontProcess(&process);
  } else {
    LOG(WARNING) << "Unable to get process for pid " << pid;
  }
}

// Takes a path to an (executable) binary and tries to provide the path to an
// application bundle containing it. It takes the outermost bundle that it can
// find (so for "/Foo/Bar.app/.../Baz.app/..." it produces "/Foo/Bar.app").
//   |exec_name| - path to the binary
//   returns - path to the application bundle, or empty on error
FilePath GetAppBundlePath(const FilePath& exec_name) {
  const char kExt[] = ".app";
  const size_t kExtLength = arraysize(kExt) - 1;

  // Split the path into components.
  std::vector<std::string> components;
  exec_name.GetComponents(&components);

  // It's an error if we don't get any components.
  if (!components.size())
    return FilePath();

  // Don't prepend '/' to the first component.
  std::vector<std::string>::const_iterator it = components.begin();
  std::string bundle_name = *it;
  DCHECK(it->length() > 0);
  // If the first component ends in ".app", we're already done.
  if (it->length() > kExtLength &&
      !it->compare(it->length() - kExtLength, kExtLength, kExt, kExtLength))
    return FilePath(bundle_name);

  // The first component may be "/" or "//", etc. Only append '/' if it doesn't
  // already end in '/'.
  if (bundle_name[bundle_name.length() - 1] != '/')
    bundle_name += '/';

  // Go through the remaining components.
  for (++it; it != components.end(); ++it) {
    DCHECK(it->length() > 0);

    bundle_name += *it;

    // If the current component ends in ".app", we're done.
    if (it->length() > kExtLength &&
        !it->compare(it->length() - kExtLength, kExtLength, kExt, kExtLength))
      return FilePath(bundle_name);

    // Separate this component from the next one.
    bundle_name += '/';
  }

  return FilePath();
}

bool SetFileBackupExclusion(const FilePath& file_path, bool exclude) {
  NSString* filePath =
      [NSString stringWithUTF8String:file_path.value().c_str()];

  // If being asked to exclude something in a tmp directory, just lie and say it
  // was done.  TimeMachine will already ignore tmp directories.  This keeps the
  // temporary profiles used by unittests from being added to the exclude list.
  // Otherwise, as /Library/Preferences/com.apple.TimeMachine.plist grows the
  // bots slow down due to reading/writing all the temporary profiles used over
  // time.

  NSString* tmpDir = NSTemporaryDirectory();
  // Make sure the temp dir is terminated with a slash
  if (tmpDir && ![tmpDir hasSuffix:@"/"])
    tmpDir = [tmpDir stringByAppendingString:@"/"];
  // '/var' is a link to '/private/var', make sure to check both forms.
  NSString* privateTmpDir = nil;
  if ([tmpDir hasPrefix:@"/var/"])
    privateTmpDir = [@"/private" stringByAppendingString:tmpDir];

  if ((tmpDir && [filePath hasPrefix:tmpDir]) ||
      (privateTmpDir && [filePath hasPrefix:privateTmpDir]) ||
      [filePath hasPrefix:@"/tmp/"] ||
      [filePath hasPrefix:@"/var/tmp/"] ||
      [filePath hasPrefix:@"/private/tmp/"] ||
      [filePath hasPrefix:@"/private/var/tmp/"]) {
    return true;
  }

  NSURL* url = [NSURL fileURLWithPath:filePath];
  // Note that we always set CSBackupSetItemExcluded's excludeByPath param
  // to true.  This prevents a problem with toggling the setting: if the file
  // is excluded with excludeByPath set to true then excludeByPath must
  // also be true when un-excluding the file, otherwise the un-excluding
  // will be ignored.
  bool success =
      CSBackupSetItemExcluded((CFURLRef)url, exclude, true) == noErr;
  if (!success)
    LOG(WARNING) << "Failed to set backup excluson for file '"
                 << file_path.value().c_str() << "'.  Continuing.";
  return success;
}

CFTypeRef GetValueFromDictionary(CFDictionaryRef dict,
                                 CFStringRef key,
                                 CFTypeID expected_type) {
  CFTypeRef value = CFDictionaryGetValue(dict, key);
  if (!value)
    return value;

  if (CFGetTypeID(value) != expected_type) {
    scoped_cftyperef<CFStringRef> expected_type_ref(
        CFCopyTypeIDDescription(expected_type));
    scoped_cftyperef<CFStringRef> actual_type_ref(
        CFCopyTypeIDDescription(CFGetTypeID(value)));
    LOG(WARNING) << "Expected value for key "
                 << base::SysCFStringRefToUTF8(key)
                 << " to be "
                 << base::SysCFStringRefToUTF8(expected_type_ref)
                 << " but it was "
                 << base::SysCFStringRefToUTF8(actual_type_ref)
                 << " instead";
    return NULL;
  }

  return value;
}

void SetProcessName(CFStringRef process_name) {
  if (!process_name || CFStringGetLength(process_name) == 0) {
    NOTREACHED() << "SetProcessName given bad name.";
    return;
  }

  if (![NSThread isMainThread]) {
    NOTREACHED() << "Should only set process name from main thread.";
    return;
  }

  // Warning: here be dragons! This is SPI reverse-engineered from WebKit's
  // plugin host, and could break at any time (although realistically it's only
  // likely to break in a new major release).
  // When 10.7 is available, check that this still works, and update this
  // comment for 10.8.

  // Private CFType used in these LaunchServices calls.
  typedef CFTypeRef PrivateLSASN;
  typedef PrivateLSASN (*LSGetCurrentApplicationASNType)();
  typedef OSStatus (*LSSetApplicationInformationItemType)(int, PrivateLSASN,
                                                          CFStringRef,
                                                          CFStringRef,
                                                          CFDictionaryRef*);

  static LSGetCurrentApplicationASNType ls_get_current_application_asn_func =
      NULL;
  static LSSetApplicationInformationItemType
      ls_set_application_information_item_func = NULL;
  static CFStringRef ls_display_name_key = NULL;

  static bool did_symbol_lookup = false;
  if (!did_symbol_lookup) {
    did_symbol_lookup = true;
    CFBundleRef launch_services_bundle =
        CFBundleGetBundleWithIdentifier(CFSTR("com.apple.LaunchServices"));
    if (!launch_services_bundle) {
      LOG(ERROR) << "Failed to look up LaunchServices bundle";
      return;
    }

    ls_get_current_application_asn_func =
        reinterpret_cast<LSGetCurrentApplicationASNType>(
            CFBundleGetFunctionPointerForName(
                launch_services_bundle, CFSTR("_LSGetCurrentApplicationASN")));
    if (!ls_get_current_application_asn_func)
      LOG(ERROR) << "Could not find _LSGetCurrentApplicationASN";

    ls_set_application_information_item_func =
        reinterpret_cast<LSSetApplicationInformationItemType>(
            CFBundleGetFunctionPointerForName(
                launch_services_bundle,
                CFSTR("_LSSetApplicationInformationItem")));
    if (!ls_set_application_information_item_func)
      LOG(ERROR) << "Could not find _LSSetApplicationInformationItem";

    const CFStringRef* key_pointer = reinterpret_cast<const CFStringRef*>(
        CFBundleGetDataPointerForName(launch_services_bundle,
                                      CFSTR("_kLSDisplayNameKey")));
    ls_display_name_key = key_pointer ? *key_pointer : NULL;
    if (!ls_display_name_key)
      LOG(ERROR) << "Could not find _kLSDisplayNameKey";

    // Internally, this call relies on the Mach ports that are started up by the
    // Carbon Process Manager.  In debug builds this usually happens due to how
    // the logging layers are started up; but in release, it isn't started in as
    // much of a defined order.  So if the symbols had to be loaded, go ahead
    // and force a call to make sure the manager has been initialized and hence
    // the ports are opened.
    ProcessSerialNumber psn;
    GetCurrentProcess(&psn);
  }
  if (!ls_get_current_application_asn_func ||
      !ls_set_application_information_item_func ||
      !ls_display_name_key) {
    return;
  }

  PrivateLSASN asn = ls_get_current_application_asn_func();
  // Constant used by WebKit; what exactly it means is unknown.
  const int magic_session_constant = -2;
  OSErr err =
      ls_set_application_information_item_func(magic_session_constant, asn,
                                               ls_display_name_key,
                                               process_name,
                                               NULL /* optional out param */);
  LOG_IF(ERROR, err) << "Call to set process name failed, err " << err;
}

}  // namespace mac_util
