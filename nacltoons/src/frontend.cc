// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "frontend.h"
#include "gameplay_scene.h"

USING_NS_CC;

bool FrontEndScene::init() {
  if (!CCScene::init())
    return false;

  // Create new (autorelease) layer and add it as child.
  CCLayer* frontend = FrontEndLayer::create();
  addChild(frontend);
  return true;
}

void FrontEndLayer::StartGame(CCObject* sender) {
  CCLog("StartGame pressed");
  CCTransitionScene* transition;
  CCDirector* director = CCDirector::sharedDirector();

  // transition to a new (autorelease) GameplayScene.
  CCScene* scene = GameplayScene::create();
  director->setDepthTest(true);
  transition = CCTransitionPageTurn::create(1.0f, scene, false);
  director->pushScene(transition);
}

bool FrontEndLayer::init() {
  if (!CCLayerColor::initWithColor(ccc4(0, 0xD8, 0x8F, 0xD8)))
    return false;

  setTouchEnabled(true);

  // Create and position the menu.
  CCLabelTTF* start_label = CCLabelTTF::create("Start Game", "Arial.ttf", 24);

  CCMenuItemLabel* start = CCMenuItemLabel::create(start_label,
      this, menu_selector(FrontEndLayer::StartGame));

  CCMenu* menu = CCMenu::create(start, NULL);
  addChild(menu);

  // Position Menu
  CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
  CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
  menu->alignItemsVertically();
  float xpos = origin.x + visibleSize.width/2;
  float ypos = origin.y + visibleSize.height/2;
  menu->setPosition(ccp(xpos, ypos));
  return true;
}
