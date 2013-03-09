// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GAME_MANAGER_H_
#define GAME_MANAGER_H_

#include "cocos2d.h"

/**
 * Tags used by the GameManager to identify scene elements. Actual values
 * here are not important as long as they are unique within the scene.
 */
enum ObjectTags {
  TAG_LAYER_LEVEL = 100,
  TAG_LAYER_UI = 101,
  TAG_LAYER_OVERLAY = 102,
};

USING_NS_CC;

/**
 * Game manager is in charge to creating scenes and transitioning
 * between them.
 */
class GameManager {
 public:
  void Restart(CCScene* scene);
  void GameOver(CCScene* scene, bool success);
  void LoadLevel(int level_number);
  static GameManager* sharedManager();
 private:
  void CreateLevel(CCScene* scene);
  GameManager() {}
  int level_number_;
};

#endif  // GAME_MANAGER_H_
