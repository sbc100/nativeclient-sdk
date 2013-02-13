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
  NODE_TAG_PHYSICS = 100,
  NODE_TAG_UI
};

USING_NS_CC;

void GameplayScene::Restart() {
  removeChild(getChildByTag(NODE_TAG_PHYSICS));
  CCLayer* physics = PhysicsLayer::create();
  addChild(physics, 2, NODE_TAG_PHYSICS);
}

bool GameplayScene::init() {
  if (!CCScene::init())
    return false;
  CCLayer* ui = UILayer::create();
  addChild(ui, 1, NODE_TAG_UI);

  CCLayer* physics = PhysicsLayer::create();
  addChild(physics, 2, NODE_TAG_PHYSICS);
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

  CCMenu* menu = CCMenu::create(restart, exit, NULL);
  addChild(menu);

  // Position the menu
  CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
  CCSize label_size = restart_label->getContentSize();
  CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
  menu->alignItemsVertically();
  float xpos = origin.x + visibleSize.width * 0.05 + label_size.width/2;
  float ypos = origin.y + visibleSize.height * 0.95 - label_size.height;
  menu->setPosition(ccp(xpos, ypos));
  return true;
}
