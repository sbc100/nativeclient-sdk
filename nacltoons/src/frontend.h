// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FRONTEND_H_
#define FRONTEND_H_

#include "cocos2d.h"

USING_NS_CC;

/**
 * Scene for displaying the frontend menu system.
 * From here the user can start a new game, view achviements, etc.
 */
class FrontEndScene : public CCScene {
 public:
  FrontEndScene() {}
  // add static create() method which encapsulates new + init
  CREATE_FUNC(FrontEndScene);
  virtual bool init();
};

/**
 * Layer that displays the frontend menu for the game.
 */
class FrontEndLayer : public CCLayerColor {
 public:
  FrontEndLayer() {}
  ~FrontEndLayer() {}

  // add static create() method which encapsulates new + init
  CREATE_FUNC(FrontEndLayer);
  virtual bool init();

 private:
  // menu callbacks
  void StartGame(CCObject* sender);
};

#endif  // FRONTEND_H_
