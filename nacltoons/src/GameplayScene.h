// Copyright (c) 2013 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef GAMEPLAY_SCENE_H
#define GAMEPLAY_SCENE_H

#include "cocos2d.h"

USING_NS_CC;

class Gameplay : public CCLayerColor
{
 public:
  Gameplay();

  ~Gameplay();

  virtual bool init();

  static CCScene* scene();

  CREATE_FUNC(Gameplay);

  virtual void ccTouchesEnded(CCSet* touches, CCEvent* event);
 protected:
  void updateGame(float dt);
};

#endif  // !GAMEPLAY_SCENE_H
