// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui_layer.h"

#include "game_manager.h"
#include "level_layer.h"

void UILayer::Restart(CCObject* sender) {
  CCLog("Restart pressed");
  CCScene* scene = static_cast<CCScene*>(getParent());
  GameManager::sharedManager()->Restart(scene);
}

void UILayer::Exit(CCObject* sender) {
  CCLog("Exit pressed");
  CCDirector::sharedDirector()->popScene();
}

#ifdef COCOS2D_DEBUG
void UILayer::ToggleDebug(CCObject* sender) {
  CCLog("ToggleDebug pressed");
  CCScene* scene = static_cast<CCScene*>(getParent());
  CCNode* layer = scene->getChildByTag(TAG_LAYER_LEVEL);
  static_cast<LevelLayer*>(layer)->ToggleDebug();
}
#endif

bool UILayer::init() {
  if (!CCLayer::init())
    return false;
  setTouchEnabled(true);

  // Create the menu.
  CCLabelTTF* restart_label = CCLabelTTF::create("Restart", "Arial.ttf", 24);
  CCLabelTTF* exit_label = CCLabelTTF::create("Exit", "Arial.ttf", 24);

  CCMenuItemLabel* restart = CCMenuItemLabel::create(
      restart_label, this, menu_selector(UILayer::Restart));

  CCMenuItemLabel* exit = CCMenuItemLabel::create(
      exit_label, this, menu_selector(UILayer::Exit));

#ifdef COCOS2D_DEBUG
  CCLabelTTF* debug_label = CCLabelTTF::create("Toggle Debug", "Arial.ttf", 24);
  CCMenuItemLabel* debug = CCMenuItemLabel::create(
      debug_label, this, menu_selector(UILayer::ToggleDebug));
  CCMenu* menu = CCMenu::create(restart, exit, debug, NULL);
#else
  CCMenu* menu = CCMenu::create(restart, exit, NULL);
#endif

  menu->alignItemsVertically();
#ifdef COCOS2D_DEBUG
  debug->setPositionX(debug->getContentSize().width / 2);
#endif
  restart->setPositionX(restart->getContentSize().width / 2);
  exit->setPositionX(exit->getContentSize().width / 2);

  addChild(menu);

  // Position the menu
  CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
  CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
  float xpos = origin.x + visibleSize.width * 0.01f;
  float ypos = origin.y + visibleSize.height / 2;
  menu->setPosition(ccp(xpos, ypos));
  return true;
}
