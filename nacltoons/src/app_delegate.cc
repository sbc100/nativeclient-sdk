// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "app_delegate.h"

#include "CCLuaEngine.h"
#include "frontend.h"
#include "LuaBox2D.h"
#include "LuaCocos2dExtensions.h"
#include "lua_level_layer.h"


USING_NS_CC;

bool AppDelegate::applicationDidFinishLaunching() {
  CCEGLView* view = CCEGLView::sharedOpenGLView();

  CCDirector* director = CCDirector::sharedDirector();
  director->setOpenGLView(view);

  // This tells cocos that our application's assets are designed for
  // native display at 600x800.  If the embed element is not this
  // size cocos will automatically scale all assets and coordinates
  // accordingly.
  view->setDesignResolutionSize(800, 600, kResolutionNoBorder);

  director->setDisplayStats(true);

  // Create lua engine
  CCLuaEngine* engine = CCLuaEngine::defaultEngine();
  assert(engine);
  if (!engine)
    return false;

  CCScriptEngineManager::sharedManager()->setScriptEngine(engine);

  // Add custom lua bindings on top of what cocos2dx provides
  CCLuaStack* stack = engine->getLuaStack();
  lua_State* lua_state = stack->getLuaState();
  assert(lua_state);
  // add box2D bindings
  tolua_LuaBox2D_open(lua_state);
  // add LevelLayer bindings
  tolua_level_layer_open(lua_state);
  // add cocos2dx extensions bindings
  tolua_extensions_open(lua_state);

  CCFileUtils* utils = CCFileUtils::sharedFileUtils();
  std::string path = utils->fullPathForFilename("loader.lua");

  // add the location of the lua file to the search path
  engine->addSearchPath(path.substr(0, path.find_last_of("/")).c_str());

  // execute loader file
  int rtn = engine->executeScriptFile(path.c_str());
  assert(!rtn);
  if (rtn)
    return false;

  // Create the initial (autorelease) scene and run with it.
  CCScene* scene = FrontEndScene::create();
  director->runWithScene(scene);
  return true;
}
