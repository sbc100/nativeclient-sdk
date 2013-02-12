// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef GAMEPLAY_SCENE_H_
#define GAMEPLAY_SCENE_H_

#include "cocos2d.h"

USING_NS_CC;

/**
 * Main gameplay scene containing the physics layer
 * that the user can interact with and a UI layer over
 * the top for menus.
 */
class Gameplay : public CCScene {
 public:
  Gameplay() {}
  ~Gameplay() {}
  CREATE_FUNC(Gameplay);
  virtual bool init();
  void Restart();
  static CCScene* scene();
};

/**
 * UI layer for menus.
 */
class UILayer : public CCLayer {
 public:
  UILayer() {}
  ~UILayer() {}
  CREATE_FUNC(UILayer);
  virtual bool init();

 private:
  // menu callbacks
  void Restart(CCObject* sender);
  void Exit(CCObject* sender);
};

#endif  // !GAMEPLAY_SCENE_H_
