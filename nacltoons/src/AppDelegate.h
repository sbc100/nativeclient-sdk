// Copyright (c) 2013 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef APP_DELEGATE_H
#define APP_DELEGATE_H

#include "cocos2d.h"

/**
 * The cocos2d-x application entry point.
 */
class AppDelegate : private cocos2d::CCApplication
{
 public:
  AppDelegate();
  virtual ~AppDelegate();

  virtual bool applicationDidFinishLaunching();
  virtual void applicationDidEnterBackground();
  virtual void applicationWillEnterForeground();
};

#endif // !APP_DELEGATE_H
