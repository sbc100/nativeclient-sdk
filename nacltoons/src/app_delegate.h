// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef APP_DELEGATE_H_
#define APP_DELEGATE_H_

#include "cocos2d.h"

enum Tags {
  TAG_BRUSH = 50,
  TAG_LEVEL_ICON = 51,
  TAG_LEVEL_ICON_SELECTED = 52,
};

/**
 * The cocos2d-x application entry point.
 */
class AppDelegate : private cocos2d::CCApplication {
 public:
  AppDelegate() {}

  virtual bool applicationDidFinishLaunching();
  virtual void applicationDidEnterBackground() {}
  virtual void applicationWillEnterForeground() {}
};

#endif  // APP_DELEGATE_H_
