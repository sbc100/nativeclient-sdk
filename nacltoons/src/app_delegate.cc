// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "app_delegate.h"
#include "frontend.h"

USING_NS_CC;

bool AppDelegate::applicationDidFinishLaunching() {
  CCEGLView* view = CCEGLView::sharedOpenGLView();

  CCDirector* director = CCDirector::sharedDirector();
  director->setOpenGLView(view);
  director->setDisplayStats(true);

  // Create the initial (autorelease) scene and run with it.
  CCScene* scene = FrontEndScene::create();
  director->runWithScene(scene);
  return true;
}
