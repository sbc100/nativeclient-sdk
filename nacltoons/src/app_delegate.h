// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef APP_DELEGATE_H_
#define APP_DELEGATE_H_

#include "cocos2d.h"

enum Tags {
  TAG_BALL = 1,
  TAG_GOAL = 2,
  TAG_STAR1 = 3,
  TAG_STAR2 = 4,
  TAG_STAR3 = 5,
  TAG_BRUSH = 6,
  TAG_LEVEL_ICON = 7,
  TAG_LEVEL_ICON_SELECTED = 8,
  TAG_OBJECTS_START = 256,
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
