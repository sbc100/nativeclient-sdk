// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef PHYSICS_LAYER_H_
#define PHYSICS_LAYER_H_

#include "cocos2d.h"
#include "Box2D/Box2D.h"

USING_NS_CC;

/**
 * Physics layer that the user interacts with.
 */
class PhysicsLayer : public CCLayerColor {
 public:
  PhysicsLayer() {}
  ~PhysicsLayer();
  CREATE_FUNC(PhysicsLayer);
  virtual bool init();
  virtual void ccTouchesEnded(CCSet* touches, CCEvent* event);

 private:
  void AddNewSpriteAtPosition(CCPoint p);
  void UpdateWorld(float dt);
  bool InitPhysics();

  CCTexture2D* sprite_texture_;
  b2World* world_;
};

#endif // !PHYSICS_LAYER_H_
