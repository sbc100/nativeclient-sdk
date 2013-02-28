// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "app_delegate.h"
#include "frontend.h"
#include "CCLuaEngine.h"

USING_NS_CC;

bool AppDelegate::applicationDidFinishLaunching() {
  CCEGLView* view = CCEGLView::sharedOpenGLView();

  CCDirector* director = CCDirector::sharedDirector();
  director->setOpenGLView(view);

  // This tells cocos that our application's assets are designed for
  // native display at 600x800.  If the embed element is not this
  // size cocos will automatically scale all assets and coordinates
  // accordingly.
  view->setDesignResolutionSize(800, 600, kResolutionNoBorder);

  director->setDisplayStats(true);

  // Create lua engine
  CCLuaEngine* engine = CCLuaEngine::defaultEngine();
  CCScriptEngineManager::sharedManager()->setScriptEngine(engine);

  // Create the initial (autorelease) scene and run with it.
  CCScene* scene = FrontEndScene::create();
  director->runWithScene(scene);
  return true;
}
