// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "level_layer.h"
#include "app_delegate.h"
#include "game_manager.h"

#include "physics_nodes/CCPhysicsSprite.h"
#include "CCLuaEngine.h"

extern "C" {
#include "lua.h"
#include "tolua++.h"
#include "lualib.h"
#include "lauxlib.h"
#include "tolua_fix.h"
}

// Pixels-to-meters ratio for converting screen coordinates
// to Box2D "meters".
#define PTM_RATIO 32

USING_NS_CC_EXT;

class Box2DCallbackHandler : public b2QueryCallback
{
 public:
  Box2DCallbackHandler(b2Vec2* test_point, CCLuaStack* lua_stack,
                       int lua_handler) :
     test_point_(*test_point),
     lua_handler_(lua_handler),
     lua_stack_(lua_stack) {}

  // Called by box2d when doing AABB testing to find bodies.
  bool ReportFixture(b2Fixture* fixture)
  {
    if (fixture->TestPoint(test_point_)) {
      b2Body* body = fixture->GetBody();
      lua_stack_->pushUserType(body, "b2Body");
      lua_stack_->executeFunctionByHandler(lua_handler_, 1);
    }

    return true; // keep looking
  }

 protected:
  // Used by b2QueryCallback to find bodies as a given location
  b2Vec2 test_point_;
  int lua_handler_;
  CCLuaStack* lua_stack_;
};

bool LevelLayer::init() {
  if (!CCLayerColor::initWithColor(ccc4(0,0x8F,0xD8,0xD8)))
    return false;

  InitPhysics();
  return true;
}

bool LevelLayer::LoadLevel(int level_number) {
  // Load level from lua file.
  LoadLua(level_number);
  CCLog("loaded level");
  setTouchEnabled(true);
  return true;
}

LevelLayer::LevelLayer() : debug_enabled_(false) {
}

LevelLayer::~LevelLayer() {
  delete box2d_world_;
#ifdef COCOS2D_DEBUG
#ifndef WIN32
  delete box2d_debug_draw_;
#endif
#endif
}

bool LevelLayer::LoadLua(int level_number) {
  CCScriptEngineManager* manager = CCScriptEngineManager::sharedManager();
  CCLuaEngine* engine = (CCLuaEngine*)manager->getScriptEngine();
  assert(engine);
  lua_stack_ = engine->getLuaStack();
  assert(lua_stack_);

  lua_stack_->pushCCObject(this, "LevelLayer");
  lua_stack_->pushInt(level_number);
  int rtn = lua_stack_->executeFunctionByName("LoadLevel", 2);
  if (rtn == -1) {
    assert(false && "level loading failed");
    return false;
  }

  return true;
}

bool LevelLayer::InitPhysics() {
  b2Vec2 gravity(0.0f, -9.8f);
  box2d_world_ = new b2World(gravity);
  box2d_world_->SetAllowSleeping(true);
  box2d_world_->SetContinuousPhysics(true);
  box2d_world_->SetContactListener(this);

#ifdef COCOS2D_DEBUG
#ifndef WIN32
  box2d_debug_draw_ = new GLESDebugDraw(PTM_RATIO);
  box2d_world_->SetDebugDraw(box2d_debug_draw_);

  uint32 flags = 0;
  flags += b2Draw::e_shapeBit;
  flags += b2Draw::e_jointBit;
  flags += b2Draw::e_centerOfMassBit;
  //flags += b2Draw::e_aabbBit;
  //flags += b2Draw::e_pairBit;
  box2d_debug_draw_->SetFlags(flags);
#endif
#endif
  return true;
}

void LevelLayer::ToggleDebug() {
  debug_enabled_ = !debug_enabled_;

  // Set visibility of all children based on debug_enabled_
  CCArray* children = getChildren();
  if (!children)
    return;
  for (uint32_t i = 0; i < children->count(); i++)
  {
    CCNode* child = static_cast<CCNode*>(children->objectAtIndex(i));
    child->setVisible(!debug_enabled_);
  }
}

CCRect CalcBoundingBox(CCSprite* sprite) {
  CCSize size = sprite->getContentSize();
  CCPoint pos = sprite->getPosition();
  return CCRectMake(pos.x - size.width, pos.y - size.height,
                    size.width, size.height/2);
}

void LevelLayer::LuaNotifyContact(b2Contact* contact,
                                    const char* function_name) {
  // Return early if lua didn't define the function_name
  lua_State* state = lua_stack_->getLuaState();
  lua_getglobal(state, function_name);
  bool is_func = lua_isfunction(state, -1);
  lua_pop(state, 1);

  if (!is_func)
    return;

  // Only send to lua collitions between body's that
  // have been tagged.
  b2Body* body1 = contact->GetFixtureA()->GetBody();
  b2Body* body2 = contact->GetFixtureB()->GetBody();
  int tag1 = (intptr_t)body1->GetUserData();
  int tag2 = (intptr_t)body2->GetUserData();
  if (!tag1 || !tag2)
    return;

  // Call the lua function callback passing in tags of the two bodies that
  // collided
  lua_stack_->pushInt(tag1);
  lua_stack_->pushInt(tag2);
  lua_stack_->executeFunctionByName(function_name, 2);
}

void LevelLayer::BeginContact(b2Contact* contact) {
  LuaNotifyContact(contact, "OnContactBegan");
}

void LevelLayer::EndContact(b2Contact* contact) {
  LuaNotifyContact(contact, "OnContactEnded");
}

void LevelLayer::LevelComplete() {
  setTouchEnabled(false);
  GameManager::sharedManager()->GameOver(true);
}

void LevelLayer::draw() {
  CCLayerColor::draw();

#ifdef COCOS2D_DEBUG
  if (debug_enabled_) {
    ccGLEnableVertexAttribs(kCCVertexAttribFlag_Position);
    kmGLPushMatrix();
    box2d_world_->DrawDebugData();
    kmGLPopMatrix();
  }
#endif
}

void LevelLayer::FindBodiesAt(b2Vec2* pos, int lua_handler) {
  b2AABB aabb;
  b2Vec2 d;
  d.Set(0.001f, 0.001f);
  aabb.lowerBound = *pos - d;
  aabb.upperBound = *pos + d;

  // Query the world for overlapping shapes.
  Box2DCallbackHandler handler(pos, lua_stack_, lua_handler);
  box2d_world_->QueryAABB(&handler, aabb);
}
