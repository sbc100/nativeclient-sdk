// Copyright (c) 2013 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "GameplayScene.h"
#include "GameOverScene.h"
#include "SimpleAudioEngine.h"
#include "physics_nodes/CCPhysicsSprite.h"

#define PTM_RATIO 32
#define PARENT_NODE_TAG 1

USING_NS_CC;
USING_NS_CC_EXT;
using namespace CocosDenshion;

Gameplay::~Gameplay()
{
}

Gameplay::Gameplay() : _spriteTexture(NULL)
{
}

CCScene* Gameplay::scene()
{
  // 'scene' is an autorelease object
  CCScene* scene = scene = CCScene::create();

  // 'layer' is an autorelease object
  Gameplay* layer = Gameplay::create();

  // add layer as a child to scene
  scene->addChild(layer);

  // return the scene
  return scene;
}

bool Gameplay::initPhysics()
{
  b2Vec2 gravity(0.0f, -9.8f);
  _world = new b2World(gravity);
  _world->SetAllowSleeping(true);
  _world->SetContinuousPhysics(true);

  // create the ground
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(0, 0);
  b2Body* groundBody = _world->CreateBody(&groundBodyDef);

  // Define the ground box shape.
  b2EdgeShape groundBox;
  CCSize s = CCDirector::sharedDirector()->getWinSize();

  // bottom
  groundBox.Set(b2Vec2(0,0), b2Vec2(s.width/PTM_RATIO,0));
  groundBody->CreateFixture(&groundBox,0);

  // top
  groundBox.Set(b2Vec2(0,s.height/PTM_RATIO), b2Vec2(s.width/PTM_RATIO,s.height/PTM_RATIO));
  groundBody->CreateFixture(&groundBox,0);

  // left
  groundBox.Set(b2Vec2(0,s.height/PTM_RATIO), b2Vec2(0,0));
  groundBody->CreateFixture(&groundBox,0);

  // right
  groundBox.Set(b2Vec2(s.width/PTM_RATIO,s.height/PTM_RATIO), b2Vec2(s.width/PTM_RATIO,0));
  groundBody->CreateFixture(&groundBox,0);
}

// on "init" you need to initialize your instance
bool Gameplay::init()
{
  bool ret = false;
  do
  {
    CC_BREAK_IF(!CCLayerColor::initWithColor(ccc4(0,0x8F,0xD8,0xD8)));
    initPhysics();

    CCSpriteBatchNode *parent = CCSpriteBatchNode::create("blocks.png", 100);
    CC_BREAK_IF(!parent);
    _spriteTexture = parent->getTexture();
    addChild(parent, 0, PARENT_NODE_TAG);

    // Create a "close" menu item with close icon, it's an auto release object.
    CCMenuItemImage* closeItem = CCMenuItemImage::create(
        "CloseNormal.png", "CloseSelected.png", this,
        menu_selector(Gameplay::exitCallback));
    CC_BREAK_IF(!closeItem);

    // Place the menu item bottom-right conner.
    CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
    CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();

    int xpos = origin.x + visibleSize.width;
    xpos -= closeItem->getContentSize().width/2;
    int ypos = origin.y + closeItem->getContentSize().height/2;
    closeItem->setPosition(ccp(xpos, ypos));

    // Create a menu with the "close" menu item, it's an auto release object.
    CCMenu* pMenu = CCMenu::create(closeItem, NULL);
    pMenu->setPosition(CCPointZero);
    CC_BREAK_IF(! pMenu);

    // Add the menu to Gameplay layer as a child layer.
    addChild(pMenu, 1);

    setTouchEnabled(true);

    schedule(schedule_selector(Gameplay::updateGame));

    //CocosDenshion::SimpleAudioEngine* soundEngine = CocosDenshion::SimpleAudioEngine::sharedEngine();
    //soundEngine->playBackgroundMusic(CCFileUtils::sharedFileUtils()->fullPathFromRelativePath("background-music-aac.wav"), true);

    ret = true;
  } while (0);

  return ret;
}

void Gameplay::addNewSpriteAtPosition(CCPoint p)
{
  CCLOG("Add sprite %0.2f x %02.f",p.x,p.y);

  // Define the dynamic body.
  //Set up a 1m squared box in the physics world
  b2BodyDef bodyDef;
  bodyDef.type = b2_dynamicBody;
  bodyDef.position.Set(p.x/PTM_RATIO, p.y/PTM_RATIO);

  b2Body *body = _world->CreateBody(&bodyDef);

  // Define another box shape for our dynamic body.
  b2PolygonShape dynamicBox;
  dynamicBox.SetAsBox(.5f, .5f);//These are mid points for our 1m box

  // Define the dynamic body fixture.
  b2FixtureDef fixtureDef;
  fixtureDef.shape = &dynamicBox;
  fixtureDef.density = 1.0f;
  fixtureDef.friction = 0.3f;
  body->CreateFixture(&fixtureDef);

  CCNode *parent = getChildByTag(PARENT_NODE_TAG);

  //We have a 64x64 sprite sheet with 4 different 32x32 images.  The following code is
  //just randomly picking one of the images
  int idx = (CCRANDOM_0_1() > .5 ? 0:1);
  int idy = (CCRANDOM_0_1() > .5 ? 0:1);
  CCPhysicsSprite *sprite = CCPhysicsSprite::createWithTexture(_spriteTexture, CCRectMake(32 * idx, 32 * idy, 32, 32));
  parent->addChild(sprite);
  sprite->setB2Body(body);
  sprite->setPTMRatio(PTM_RATIO);
  sprite->setPosition( ccp( p.x, p.y) );
}

void Gameplay::exitCallback(CCObject* pSender)
{
  // Callback from the exit button
  CCDirector::sharedDirector()->end();
}

// cpp with cocos2d-x
void Gameplay::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
  for (CCSetIterator it = touches->begin(); it != touches->end(); it++)
  {
    CCTouch* touch = (CCTouch*)(*it);
    if (!touch)
      break;

    CCPoint location = touch->getLocation();
    CCLog("touch x:%f, y:%f", location.x, location.y);

    addNewSpriteAtPosition(location);
  }
}

#define VELOCITY_ITERATIONS 8
#define POS_ITERATIONS 1

void Gameplay::updateGame(float dt) {
  // Instruct the world to perform a single step of simulation. It is
  // generally best to keep the time step and iterations fixed.
  _world->Step(dt, VELOCITY_ITERATIONS, POS_ITERATIONS);
}

void Gameplay::registerWithTouchDispatcher() {
  CCDirector::sharedDirector()->getTouchDispatcher()->addStandardDelegate(this, 0);
}
