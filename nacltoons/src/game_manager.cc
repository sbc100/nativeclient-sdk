// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "game_manager.h"
#include "level_layer.h"
#include "ui_layer.h"

USING_NS_CC;

GameManager* GameManager::sharedManager()
{
  static GameManager* shared_manager = NULL;
  if (!shared_manager)
    shared_manager = new GameManager();
  return shared_manager;
}

void GameManager::CreateLevel(CCScene* scene)
{
  CCLayer* level = LevelLayer::create(level_number_);
  scene->addChild(level, 1, TAG_LAYER_LEVEL);
}

void GameManager::Restart(CCScene* scene)
{
  scene->removeChild(scene->getChildByTag(TAG_LAYER_LEVEL));
  scene->removeChild(scene->getChildByTag(TAG_LAYER_OVERLAY));
  CreateLevel(scene);
}

void GameManager::LoadLevel(int level_number)
{
  CCDirector* director = CCDirector::sharedDirector();
  CCTransitionScene* transition;
  level_number_ = level_number;
  CCScene* scene = CCScene::create();
  CCLayer* ui = UILayer::create();
  scene->addChild(ui, 3, TAG_LAYER_UI);

  CreateLevel(scene);

  director->setDepthTest(true);
  transition = CCTransitionPageTurn::create(1.0f, scene, false);
  director->pushScene(transition);
}

void GameManager::GameOver(CCScene* scene, bool success) {
  CCSize visible_size = CCDirector::sharedDirector()->getVisibleSize();

  // Create a black overlay layer with success/failure message
  CCLayer* overlay = CCLayerColor::create(ccc4(0x0, 0x0, 0x0, 0x0));
  const char* text = success ? "Success!" : "Failure!";
  CCLabelTTF* label = CCLabelTTF::create(text, "Arial.ttf", 24);
  label->setPosition(ccp(visible_size.width/2, visible_size.height/2));
  overlay->addChild(label);
  scene->addChild(overlay, 2, TAG_LAYER_OVERLAY);

  // Face the overlay layer into to 50%
  CCActionInterval* fadein = CCFadeTo::create(0.5f, 0x7F);
  overlay->runAction(fadein);
}
