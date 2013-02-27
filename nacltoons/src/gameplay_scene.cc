// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "gameplay_scene.h"
#include "physics_layer.h"

/**
 * Tags used by the GameplayScene to identify child elements. Actual values
 * here are not important as long as they are unique within the scene.
 */
enum ObjectTags {
  TAG_LAYER_PHYSICS = 100,
  TAG_LAYER_UI = 101,
  TAG_LAYER_OVERLAY = 102,
};

USING_NS_CC;

void GameplayScene::Restart() {
  removeChild(getChildByTag(TAG_LAYER_PHYSICS));
  removeChild(getChildByTag(TAG_LAYER_OVERLAY));
  CCLayer* physics = PhysicsLayer::create();
  addChild(physics, 1, TAG_LAYER_PHYSICS);
}

void GameplayScene::GameOver(bool success) {
  CCSize visible_size = CCDirector::sharedDirector()->getVisibleSize();

  // Create a black overlay layer with success/failure message
  CCLayer* overlay = CCLayerColor::create(ccc4(0x0, 0x0, 0x0, 0x0));
  const char* text = success ? "Success!" : "Failure!";
  CCLabelTTF* label = CCLabelTTF::create(text, "Arial.ttf", 24);
  label->setPosition(ccp(visible_size.width/2, visible_size.height/2));
  overlay->addChild(label);
  addChild(overlay, 2, TAG_LAYER_OVERLAY);

  // Face the overlay layer into to 50%
  CCActionInterval* fadein = CCFadeTo::create(0.5f, 0x7F);
  overlay->runAction(fadein);
}

bool GameplayScene::init() {
  if (!CCScene::init())
    return false;
  CCLayer* ui = UILayer::create();
  addChild(ui, 3, TAG_LAYER_UI);

  Restart();
  return true;
}

void UILayer::Restart(CCObject* sender) {
  CCLog("Restart pressed");
  GameplayScene* scene = static_cast<GameplayScene*>(getParent());
  scene->Restart();
}

void UILayer::Exit(CCObject* sender) {
  CCLog("Exit pressed");
  CCDirector::sharedDirector()->popScene();
}

#ifdef COCOS2D_DEBUG
void UILayer::ToggleDebug(CCObject* sender) {
  CCLog("ToggleDebug pressed");
  GameplayScene* scene = static_cast<GameplayScene*>(getParent());
  CCNode* physics = scene->getChildByTag(TAG_LAYER_PHYSICS);
  static_cast<PhysicsLayer*>(physics)->ToggleDebug();
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
