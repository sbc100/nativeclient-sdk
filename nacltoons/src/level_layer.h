// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef LEVEL_LAYER_H_
#define LEVEL_LAYER_H_

#include "cocos2d.h"
#include "CCLuaStack.h"
#include "Box2D/Box2D.h"

#ifdef COCOS2D_DEBUG
#include "GLES-Render.h"
#endif

USING_NS_CC;

typedef std::vector<cocos2d::CCPoint> PointList;

/**
 * Lavel layer in which gameplay takes place.  This layer contains
 * the box2d world simulation.
 */
class LevelLayer : public CCLayerColor,
                   public b2ContactListener {
 public:
  LevelLayer();
  ~LevelLayer();

  CREATE_FUNC(LevelLayer);

  virtual bool init();
  virtual void draw();

  b2World* GetWorld() { return box2d_world_; }

  b2Body* FindBodyAt(b2Vec2* pos);

  void ToggleDebug();
  bool LoadLevel(int level_number);

  // Called by box2d when contacts start
  void BeginContact(b2Contact* contact);

  // Called by box2d when contacts finish
  void EndContact(b2Contact* contact);

  // Methods that are exposed to / called by lua the lua
  // script.
  void LevelComplete();

 protected:
  // Called by BeginContact and EndContact to nofify Lua code when box2d
  // contacts start and finish.
  void LuaNotifyContact(b2Contact* contact, const char* function_name);

  bool LoadLua(int level_number);

  bool InitPhysics();
  void UpdateWorld(float dt);

 private:
  // Box2D physics world
  b2World* box2d_world_;

#ifdef COCOS2D_DEBUG
  // Debug drawing support for Box2D.
  GLESDebugDraw* box2d_debug_draw_;
#endif

  // Flag to enable drawing of Box2D debug data.
  bool debug_enabled_;

  CCLuaStack* lua_stack_;
};

#endif  // LEVEL_LAYER_H_
