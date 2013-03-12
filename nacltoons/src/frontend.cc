// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "frontend.h"
#include "app_delegate.h"
#include "game_manager.h"

#include "CCLuaEngine.h"

USING_NS_CC;

const int kFontSize = 24;
const int kFontSizeLarge = 48;
const char* kFontName = "Arial.ttf";

bool FrontEndScene::init() {
  if (!CCScene::init())
    return false;

  // Create new (autorelease) layer and add it as child.
  CCLayer* frontend = FrontEndLayer::create();
  addChild(frontend);
  return true;
}

void LayoutMenu(CCMenu* menu, uint row_size, CCPoint padding)
{
  CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
  CCSize visible_size = CCDirector::sharedDirector()->getVisibleSize();
  CCArray* items = menu->getChildren();

  CCSize item_size = ((CCMenuItem*)items->objectAtIndex(0))->getContentSize();
  float w = item_size.width;
  float h = item_size.height;

  uint num_rows = (items->count() + row_size - 1) / row_size;
  float menu_height = num_rows * (h + padding.y);
  float startY = (visible_size.height - menu_height) / 2;

  uint item_index = 0;
  for (uint i = 0; i < num_rows; i++)
  {
    int num_cols = MIN(row_size, items->count() - (i * row_size));

    float row_width = num_cols * w + padding.x * (num_cols - 1);
    float startX = (visible_size.width - row_width)/2;
    float y = startY + (num_rows - i - 1) * (padding.y + h);

    for (int j = 0; j < num_cols; j++)
    {
      CCMenuItem *item = (CCMenuItem *)items->objectAtIndex(item_index);
      item->setAnchorPoint(ccp(0,0));
      item->setPosition(ccp(startX, y));
      CCLog("Setting pos of menu item %d: %.fx%.f", item_index, startX, y);
      startX += padding.x + w;
      item_index++;
    }
  }

  assert(item_index == items->count());

  menu->setPosition(ccp(origin.x, origin.y));
}

void FrontEndLayer::LevelSelected(CCObject* sender) {
  int level = ((CCMenuItem*)sender)->getTag();
  CCLog("LevelSelected: %d", level);
  GameManager::sharedManager()->LoadLevel(level);
}

void FrontEndLayer::AddChildAligned(CCNode* node, int height) {
  CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
  CCSize visible_size = CCDirector::sharedDirector()->getVisibleSize();
  CCSize node_size = node->getContentSize();
  // A negative height means an offet from the top of the visible area
  if (height < 0)
    height += visible_size.height;
  float xpos = origin.x + visible_size.width / 2;
  float ypos = origin.y + height - node_size.height / 2;
  node->setPosition(ccp(xpos, ypos));
  addChild(node);
}

void FrontEndLayer::StartGame(CCObject* sender) {
  CCLog("StartGame pressed");

  // Load the game.def file
  GameManager::sharedManager()->LoadGame("sample_game");

  // Remove the old menu that sent us here
  CCNode* old_menu = ((CCMenuItem*)sender)->getParent();
  removeChild(old_menu);

  // Call the LevelCount global lua function from loader.lua
  CCScriptEngineManager* manager = CCScriptEngineManager::sharedManager();
  CCLuaEngine* engine = (CCLuaEngine*)manager->getScriptEngine();
  CCLuaStack* stack = engine->getLuaStack();

  stack->pushCCObject(this, "CCLayerColor");
  int level_count = stack->executeFunctionByName("LevelCount", 1);
  CCLog("found %d levels", level_count);
  assert(level_count > 0);

  CCLabelTTF* start_label = CCLabelTTF::create("Select Level", kFontName,
                                               kFontSizeLarge);
  //start_label->setHorizontalAlignment(kCCTextAlignmentCenter);
  AddChildAligned(start_label, -100);

  // Create the level selection menu
  CCMenu* menu = CCMenu::create();
  CCLabelTTF* label;
  CCMenuItemSprite* item;
  CCSprite* normal = (CCSprite*)getChildByTag(TAG_LEVEL_ICON);
  CCSprite* selected = (CCSprite*)getChildByTag(TAG_LEVEL_ICON_SELECTED);
  removeChild(normal);
  removeChild(selected);

  CCSize icon_size = normal->getContentSize();
  CCPoint label_pos = ccp(icon_size.width/2, icon_size.height/2);

  // For each level create a menu item and a textual label
  for (int i = 0; i < level_count; i++)
  {
    char label_string[10];
    sprintf(label_string, "%d", i + 1) ;
    item = CCMenuItemSprite::create(
        CCSprite::createWithTexture(normal->getTexture()),
        CCSprite::createWithTexture(selected->getTexture()),
        this, menu_selector(FrontEndLayer::LevelSelected));
    label = CCLabelTTF::create(label_string, kFontName, kFontSizeLarge);
    label->setPosition(label_pos);
    item->addChild(label);
    item->setTag(i + 1);
    menu->addChild(item);
  }

  // Layout the level menu in rows
  LayoutMenu(menu, 2, ccp(20, 20));
  addChild(menu);
}

bool FrontEndLayer::init() {
  if (!CCLayerColor::initWithColor(ccc4(0, 0xD8, 0x8F, 0xD8)))
    return false;

  setTouchEnabled(true);

  // Create and position the menu.
  CCLabelTTF* start_label = CCLabelTTF::create("Start Game", kFontName,
                                               kFontSize);

  CCMenuItemLabel* start = CCMenuItemLabel::create(start_label,
      this, menu_selector(FrontEndLayer::StartGame));

  CCMenu* menu = CCMenu::create(start, NULL);
  addChild(menu);

  // Position Menu
  CCSize visible_size = CCDirector::sharedDirector()->getVisibleSize();
  CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
  menu->alignItemsVertically();
  float xpos = origin.x + visible_size.width/2;
  float ypos = origin.y + visible_size.height/2;
  menu->setPosition(ccp(xpos, ypos));
  return true;
}
