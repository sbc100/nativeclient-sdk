// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef PHYSICS_LAYER_H_
#define PHYSICS_LAYER_H_

#include "cocos2d.h"
#include "Box2D/Box2D.h"

#ifdef COCOS2D_DEBUG
#include "GLES-Render.h"
#endif

USING_NS_CC;

typedef std::vector<cocos2d::CCPoint> PointList;

/**
 * Physics layer that the user interacts with.
 */
class PhysicsLayer : public CCLayerColor {
 public:
  PhysicsLayer();
  ~PhysicsLayer();

  // Add static create() method which encapsulates new + init
  CREATE_FUNC(PhysicsLayer);

  virtual bool init();
  virtual void draw();
  virtual bool ccTouchBegan(CCTouch* touch, CCEvent* event);
  virtual void ccTouchMoved(CCTouch* touch, CCEvent* event);
  virtual void ccTouchEnded(CCTouch* touch, CCEvent* event);
  virtual void registerWithTouchDispatcher();

  b2World* GetWorld() { return box2d_world_; }

  // Add the current (most recently created) instance.
  // Unfortunately this is needed by the lua bindings so the it can access the
  // GetWorld method.
  // TODO(sbc): find a way to do without this.
  static PhysicsLayer* GetCurrent() { return current_instance_; }

#ifdef COCOS2D_DEBUG
  void ToggleDebug();
#endif

 private:
  // Called by ccTouchesMoved to draw between two points
  void DrawLine(CCPoint& start, CCPoint& end);
  // Called by ccTouchesBegan to draw a single point.
  void DrawPoint(CCPoint& location);
  // Clamp brush location to within the visible area
  void ClampBrushLocation(CCPoint& point);

  void AddLineToBody(b2Body *body, CCPoint start, CCPoint end);
  void AddSphereToBody(b2Body *body, CCPoint* location);
  void AddShapeToBody(b2Body *body, b2Shape* shape);

  // Create a new render target for drawing into
  void CreateRenderTarget();

  // Create a physics object based on the points that were drawn.
  b2Body* CreatePhysicsBody();
  CCSprite* CreatePhysicsSprite(b2Body* body);
  void UpdateWorld(float dt);
  bool InitPhysics();
  bool LoadLua();
  void LevelComplete(CCNode* sender);

 private:
  static PhysicsLayer* current_instance_;

  // Brush texture used for drawing shapes
  CCSprite* brush_;
  float brush_radius_;
  bool stars_collected_[3];
  bool goal_reached_;

  // The touch ID that is drawing the current shape.  Events from
  // other touches are ignored while this has a valid value.
  int current_touch_id_;

  // Render target that shapes are drawn into
  CCRenderTexture* render_target_;

  // Box2D physics world
  b2World* box2d_world_;

  // Density given to new physics objects
  float box2d_density_;
  // Restitution given to new physics objects
  float box2d_restitution_;
  // Friction given to new physics objects
  float box2d_friction_;

  // The list of points currently being drawn by the user
  PointList points_being_drawn_;

#ifdef COCOS2D_DEBUG
  // Debug drawing support for Box2D.
  GLESDebugDraw* box2d_debug_draw_;
  // Flag to enable drawing of Box2D debug data.
  bool debug_enabled_;
#endif
};

#endif  // PHYSICS_LAYER_H_
