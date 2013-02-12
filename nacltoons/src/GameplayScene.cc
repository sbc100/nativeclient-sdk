// Copyright (c) 2013 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "GameplayScene.h"

USING_NS_CC;

Gameplay::~Gameplay()
{
}

Gameplay::Gameplay()
{
}

CCScene* Gameplay::scene()
{
  CCScene* scene = CCScene::create();
  Gameplay* layer = Gameplay::create();
  scene->addChild(layer);
  return scene;
}

bool Gameplay::init()
{
  CCLayerColor::initWithColor(ccc4(0,0x8F,0xD8,0xD8));
  setTouchEnabled(true);
  schedule(schedule_selector(Gameplay::updateGame));
  return true;
}

void Gameplay::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
  for (CCSetIterator it = touches->begin(); it != touches->end(); it++)
  {
    CCTouch* touch = (CCTouch*)(*it);
    if (!touch)
      break;

    CCPoint location = touch->getLocation();
    CCLog("touch x:%f, y:%f", location.x, location.y);
  }
}

void Gameplay::updateGame(float dt) {
}
