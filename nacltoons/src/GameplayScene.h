// Copyright (c) 2013 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef GAMEPLAY_SCENE_H
#define GAMEPLAY_SCENE_H

#include "cocos2d.h"
#include "Box2D/Box2D.h"

using namespace cocos2d;

class Gameplay : public CCLayerColor
{
 public:
  Gameplay();

  ~Gameplay();

  virtual bool init();

  // there's no 'id' in cpp, so we recommand to return the exactly class pointer
  static CCScene* scene();

  // add create() function for this class.
  CREATE_FUNC(Gameplay);

  // override:
  virtual void registerWithTouchDispatcher();

  // override: called when a touch event occurs on the layer
  virtual void ccTouchesEnded(CCSet* touches, CCEvent* event);
 protected:
  // Callback from the exit button.
  void exitCallback(CCObject* pSender);

  // Callback for sprite movement complete.
  void spriteMoveFinished(CCNode* sender);

  // called on a 1 second timer
  void gameLogic(float dt);

  // called evert frame to detect collisions
  void updateGame(float dt);

  // called every second gameLogic
  void addTarget();

 protected:
  bool initPhysics();
  void addNewSpriteAtPosition(CCPoint p);
  // The Box2D physics instance
  b2World* _world;
  // The rigid platform
  b2Body* _platformBody;
  CCTexture2D* _spriteTexture;
};

#endif  // !GAMEPLAY_SCENE_H
