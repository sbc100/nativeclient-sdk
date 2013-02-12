// Copyright (c) 2013 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "AppDelegate.h"
#include "GameplayScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

bool AppDelegate::applicationDidFinishLaunching() {
  CCEGLView* view = CCEGLView::sharedOpenGLView();

  CCDirector* director = CCDirector::sharedDirector();
  director->setOpenGLView(view);
  director->setDisplayStats(true);

  CCScene* scene = Gameplay::scene();
  director->runWithScene(scene);
  return true;
}
