// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef UI_LAYER_H_
#define UI_LAYER_H_

#include "cocos2d.h"

USING_NS_CC;

/**
 * UI layer for displaying menu and information overlays on
 * top of the running game.
 */
class UILayer : public CCLayer {
 public:
  UILayer() {}
  ~UILayer() {}
  // add static create() method which encapsulates new + init
  CREATE_FUNC(UILayer);
  virtual bool init();

 private:
  // menu callbacks
  void ToggleDebug(CCObject* sender);
  void Restart(CCObject* sender);
  void Exit(CCObject* sender);
};

#endif  // UI_LAYER_H_
