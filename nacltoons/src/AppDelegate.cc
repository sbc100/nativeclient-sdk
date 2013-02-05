// Copyright (c) 2013 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "AppDelegate.h"
#include "GameplayScene.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;

AppDelegate::AppDelegate() {
}

AppDelegate::~AppDelegate() {
}

bool AppDelegate::applicationDidFinishLaunching() {
  // initialize director
  CCDirector* director = CCDirector::sharedDirector();

  // initialize GLES
  CCEGLView* view = CCEGLView::sharedOpenGLView();

  director->setOpenGLView(view);
  CCSize screenSize = view->getFrameSize();
  CCSize designSize = CCSizeMake(480, 320);

  if (screenSize.height > 320)
  {
    CCFileUtils::sharedFileUtils()->setResourceDirectory("hd");
    director->setContentScaleFactor(640.0f/designSize.height);
  }
  else
  {
    CCFileUtils::sharedFileUtils()->setResourceDirectory("sd");
    director->setContentScaleFactor(320.0f/designSize.height);
  }

  view->setDesignResolutionSize(designSize.width, designSize.height,
                                kResolutionNoBorder);

  // turn on display FPS
  director->setDisplayStats(true);

  // set FPS. the default value is 1.0/60 if you don't call this
  director->setAnimationInterval(1.0 / 60);

  // create a scene. it's an autorelease object
  CCScene* scene = Gameplay::scene();

  // run
  director->runWithScene(scene);
  return true;
}

// This function will be called when the app is inactive.
void AppDelegate::applicationDidEnterBackground() {
  CCDirector::sharedDirector()->stopAnimation();
  CocosDenshion::SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
  CCDirector::sharedDirector()->startAnimation();
  CocosDenshion::SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
}
