// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "physics_layer.h"
#include "physics_nodes/CCPhysicsSprite.h"

// Pixels-to-meters ratio for converting screen coordinates
// to Box2D "meters".
#define PTM_RATIO 32
#define SCREEN_TO_WORLD(n) ((n) / PTM_RATIO)
#define WORLD_TO_SCREEN(n) ((n) * PTM_RATIO)
#define VELOCITY_ITERATIONS 8
#define POS_ITERATIONS 1

#define SPRITE_BATCH_NODE_TAG 99
#define MAX_SPRITES 100

USING_NS_CC_EXT;

bool PhysicsLayer::init() {
  if (!CCLayerColor::initWithColor(ccc4(0,0x8F,0xD8,0xD8)))
    return false;

  setTouchEnabled(true);
  InitPhysics();

  CCSpriteBatchNode* batch;
  batch = CCSpriteBatchNode::create("blocks.png", MAX_SPRITES);
  addChild(batch, 0, SPRITE_BATCH_NODE_TAG);

  // cache texture pointer for creating sprites
  sprite_texture_ = batch->getTexture();

  // script physics updates each frame
  schedule(schedule_selector(PhysicsLayer::UpdateWorld));
  return true;
}

PhysicsLayer::~PhysicsLayer() {
  delete world_;
}

bool PhysicsLayer::InitPhysics() {
  b2Vec2 gravity(0.0f, -9.8f);
  world_ = new b2World(gravity);
  world_->SetAllowSleeping(true);
  world_->SetContinuousPhysics(true);

  // create the ground
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0, 0);
  b2Body* groundBody = world_->CreateBody(&groundBodyDef);

  CCSize win_size = CCDirector::sharedDirector()->getWinSize();
  int world_width = SCREEN_TO_WORLD(win_size.width);
  int world_height = SCREEN_TO_WORLD(win_size.height);

  // Define the ground box shape.
  b2EdgeShape groundBox;
  // bottom
  groundBox.Set(b2Vec2(0, 0), b2Vec2(world_width, 0));
  groundBody->CreateFixture(&groundBox, 0);
  // top
  groundBox.Set(b2Vec2(0, world_height), b2Vec2(world_width, world_height));
  groundBody->CreateFixture(&groundBox, 0);
  // left
  groundBox.Set(b2Vec2(0, world_height), b2Vec2(0,0));
  groundBody->CreateFixture(&groundBox, 0);
  // right
  groundBox.Set(b2Vec2(world_width, world_height), b2Vec2(world_width, 0));
  groundBody->CreateFixture(&groundBox, 0);
}

void PhysicsLayer::UpdateWorld(float dt) {
   world_->Step(dt, VELOCITY_ITERATIONS, POS_ITERATIONS);
}

void PhysicsLayer::ccTouchesEnded(CCSet* touches, CCEvent* event) {
  for (CCSetIterator it = touches->begin(); it != touches->end(); it++) {
    CCTouch* touch = (CCTouch*)(*it);
    if (!touch)
      break;

    CCPoint location = touch->getLocation();
    //CCLOG("PhysicsLayer touch x:%.f, y:%.f", location.x, location.y);
    AddNewSpriteAtPosition(location);
  }
}

void PhysicsLayer::AddNewSpriteAtPosition(CCPoint p) {
  CCLOG("Add sprite %0.fx%.f", p.x, p.y);

  // Create a new physics body at the given position.
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position.Set(SCREEN_TO_WORLD(p.x), SCREEN_TO_WORLD(p.y));
  b2Body* body = world_->CreateBody(&body_def);

  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(.5f, .5f);

  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 1.0f;
  fixtureDef.friction = 0.3f;
  body->CreateFixture(&fixtureDef);

  // Create new sprite an link it to the physics body.
  CCPhysicsSprite* sprite;
  sprite = CCPhysicsSprite::createWithTexture(sprite_texture_,
                                              CCRectMake(0, 0, 32, 32));

  // Find the sprite batch node and attach new sprite to it.
  CCNode* parent = getChildByTag(SPRITE_BATCH_NODE_TAG);
  parent->addChild(sprite);
  sprite->setB2Body(body);
  sprite->setPTMRatio(PTM_RATIO);
  sprite->setPosition(ccp(p.x, p.y));
}
