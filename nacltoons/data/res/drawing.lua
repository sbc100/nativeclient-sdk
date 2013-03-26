-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

-- Functions for drawing new sprites using a brush.
-- This module defines a single 'drawing' global containing the following
-- functions:
--   - SetBrush
--   - CreateShape
--   - CreateSprite
--   - DrawStartPoint
--   - DrawEndPoint
--   - AddLineToShape
--   - OnTouchBegan
--   - OnTouchMoved
--   - OnTouchEnded

require 'util'

drawing = {}

-- Brush information (set by SetBrush)
local brush_node
local brush_thickness

-- Constant for grouping physics bodies
local MAIN_CATEGORY = 0x1
local DRAWING_CATEGORY = 0x2

-- Local state for default touch handlers
local current_shape = nil
local current_tag = 99 -- util.tags.TAG_DYNAMIC_START
local last_pos = nil
local brush_color = ccc3(255, 100, 100)

--- Create a b2Vec from a lua list containing 2 elements.
-- This is used to convert point data from .def files directly
-- to the box2dx coordinate system
local function b2VecFromLua(point)
    return util.b2VecFromCocos(util.PointFromLua(point))
end

--- Create a fixed pivot point between the world and the given body.
local function CreatePivot(anchor, body)
    local anchor_point = util.b2VecFromCocos(anchor)

    -- create a new fixed body to pivot against
    local ground_def = b2BodyDef:new_local()
    ground_def.position = anchor_point
    local ground_body = level_obj.world:CreateBody(ground_def);

    -- create the pivot joint
    local joint_def = b2RevoluteJointDef:new_local()
    joint_def:Initialize(ground_body, body, anchor_point)
    local joint = level_obj.world:CreateJoint(joint_def)
end

local function AddShapeToBody(body, shape, sensor)
    local fixture_def = b2FixtureDef:new_local()
    fixture_def.shape = shape
    fixture_def.density = 1.0
    fixture_def.friction = 0.5
    fixture_def.restitution = 0.3
    fixture_def.isSensor = sensor
    return body:CreateFixture(fixture_def)
end

-- Add a new circle/sphere fixture to a body and return the new fixture
local function AddSphereToBody(body, location, radius, sensor)
    local sphere = b2CircleShape:new_local()
    sphere.m_radius = util.ScreenToWorld(radius)
    sphere.m_p.x = util.ScreenToWorld(location.x) - body:GetPosition().x
    sphere.m_p.y = util.ScreenToWorld(location.y) - body:GetPosition().y
    return AddShapeToBody(body, sphere, sensor)
end

-- Add a new line/box fixture to a body and return the new fixture
local function AddLineToShape(sprite, from, to, color)
    -- calculate length and angle of line based on start and end points
    local body = sprite:getB2Body()
    local length = ccpDistance(from, to);
    local dist_x = to.x - from.x
    local dist_y = to.y - from.y
    util.Log('Create line at: ' .. math.floor(from.x) .. 'x' .. math.floor(from.y) .. ' len=' .. math.floor(length))

    -- create fixture
    local relative_start_x = from.x - sprite:getPositionX()
    local relative_start_y = from.y - sprite:getPositionY()
    local center = b2Vec2:new_local(util.ScreenToWorld(relative_start_x + dist_x/2),
                                    util.ScreenToWorld(relative_start_y + dist_y/2))
    local shape = b2PolygonShape:new_local()
    local angle = math.atan2(dist_y, dist_x)
    shape:SetAsBox(util.ScreenToWorld(length/2), util.ScreenToWorld(brush_thickness),
                   center, angle)
    local fixture = AddShapeToBody(body, shape, false)

    -- Now create the  visible CCPhysicsSprite that the body is attached to
    sprite:setColor(color)
    sprite:setPTMRatio(util.PTM_RATIO)

    -- And add a sequence of non-physics sprites as children of the first
    local dist = CCPointMake(dist_x, dist_y)
    local num_children = length / brush_thickness
    local inc_x = dist_x / num_children
    local inc_y = dist_y / num_children
    local child_location = ccp(relative_start_x + brush_thickness, relative_start_y + brush_thickness)
    for i = 1,num_children do
        child_location.x = child_location.x + inc_x
        child_location.y = child_location.y + inc_y

        local child_sprite = CCSprite:createWithTexture(brush_tex)
        child_sprite:setPosition(child_location)
        child_sprite:setColor(color)
        sprite:addChild(child_sprite)
    end

    return fixture
end

--- Create a line between two points.
-- Uses a sequence of brush sprites an a single box2d rect.
local function CreateLine(from, to, objdef)
    -- create body
    local body_def = b2BodyDef:new_local()
    if objdef.dynamic == true then
        body_def.type = b2_dynamicBody
    end
    local body = level_obj.world:CreateBody(body_def)

    if objdef.anchor then
        CreatePivot(objdef.anchor, body)
    end

    if objdef.color then
        color = ccc3(objdef.color[1], objdef.color[2], objdef.color[3])
    else
        color = ccc3(255, 255, 255)
    end

    local sprite = CCPhysicsSprite:createWithTexture(brush_tex)
    sprite:setB2Body(body);
    AddLineToShape(sprite, from, to, color)
    return sprite
end

local function NewPhysicsSprite(sprite, location)
    local body_def = b2BodyDef:new_local()
    local body = level_obj.world:CreateBody(body_def)
    sprite:setB2Body(body)
    sprite:setPTMRatio(util.PTM_RATIO)
    sprite:setPosition(location)
    return body
end

-- Set the collision group for a fixture
local function SetCategory(fixture, category)
    local filter = fixture:GetFilterData()
    filter.categoryBits = category
    filter.maskBits = category
    fixture:SetFilterData(filter)
end

--- Make the a body dynamic and put it in the default collision group
local function MakeBodyDynamic(body)
    body:SetType(b2_dynamicBody)
    local fixture = body:GetFixtureList()
    while fixture do
        SetCategory(fixture, MAIN_CATEGORY)
        fixture = fixture:GetNext()
    end
end

--- Set brush texture for subsequent draw operations
drawing.SetBrush = function (brush)
    -- calculate thickness based on brush sprite size
    brush_node = brush
    brush_tex = brush:getTexture()
    local brush_size = brush_tex:getContentSizeInPixels()
    brush_thickness = math.max(brush_size.height/2, brush_size.width/2);
end

--- Draw a shape described by a given shape def.
-- This creates physics sprites and accosiated box2d bodies for
-- the shape.
drawing.CreateShape = function(shape_def)
    if shape_def.type == 'line' then
        local start = util.PointFromLua(shape_def.start)
        local finish = util.PointFromLua(shape_def.finish)
        if shape_def.anchor then
            shape_def.anchor = util.PointFromLua(shape_def.anchor)
        end
        local line = CreateLine(start, finish, shape_def)
        brush_node:addChild(line, 1)
    elseif shape_def.type == 'edge' then
        local body_def = b2BodyDef:new_local()
        local body = level_obj.world:CreateBody(body_def)
        local b2shape = b2EdgeShape:new_local()
        b2shape:Set(b2VecFromLua(shape_def.start), b2VecFromLua(shape_def.finish))
        body:CreateFixture(b2shape, 0)
    else
        assert(false)
    end
end

--- Create a physics sprite at a fiven location with a given image
drawing.CreateSprite = function(sprite_def)
    local pos = util.PointFromLua(sprite_def.pos)
    -- util.Log('Create sprite [tag=' .. sprite_def.tag .. ' image=' .. sprite_def.image .. ']: ' ..
    --    math.floor(pos.x) .. 'x' .. math.floor(pos.y))
    local image = game_obj.assets[sprite_def.image]
    local sprite = CCPhysicsSprite:create(image)
    local body = NewPhysicsSprite(sprite, pos)
    body:SetUserData(sprite_def.tag)
    if not sprite_def.sensor then
         body:SetType(b2_dynamicBody)
    end

    AddSphereToBody(body, pos, sprite:boundingBox().size.height/2, sprite_def.sensor)
    return sprite
end

--- Create a single circlular point with the brush.
-- This is used to start shapes that the use draws.  The starting
-- point contains the box2d body for the shape.
drawing.DrawStartPoint = function(location, color)
    -- Add visible sprite
    local sprite = CCPhysicsSprite:createWithTexture(brush_tex)
    local body = NewPhysicsSprite(sprite, location)
    sprite:setColor(color)
    brush_node:addChild(sprite)

    -- Add collision info
    local fixture = AddSphereToBody(body, location, sprite:boundingBox().size.height/2, false)
    SetCategory(fixture, DRAWING_CATEGORY)
    return sprite
end

drawing.DrawEndPoint = function(sprite, location, color)
    -- Add visible sprite
    local child_sprite = CCSprite:createWithTexture(brush_tex)
    local relative_x = location.x - sprite:getPositionX()
    local relative_y = location.y - sprite:getPositionY()
    local brush_size = brush_tex:getContentSizeInPixels()
    child_sprite:setPosition(ccp(relative_x + brush_size.width/2, relative_y + brush_size.height/2))
    child_sprite:setColor(color)
    sprite:addChild(child_sprite)

    -- Add collision info
    local body = sprite:getB2Body()
    local fixture = AddSphereToBody(body, location, sprite:boundingBox().size.height/2, false)
    SetCategory(fixture, DRAWING_CATEGORY)

    MakeBodyDynamic(body)
end

drawing.AddLineToShape = function(sprite, from, to, color)
    fixture = AddLineToShape(sprite, from, to, color)
    SetCategory(fixture, DRAWING_CATEGORY)
end

--- Sample OnTouchMoved for drawing-based games.  For bespoke drawing behaviour
-- clone and modify this code.
drawing.OnTouchBegan = function(x, y)
   --- ignore touches if we are already drawing something
   if current_shape then
       return false
   end

   -- create initial sphere to represent start of shape
   last_pos = ccp(x, y)
   current_shape = drawing.DrawStartPoint(last_pos, brush_color)
   current_shape:setTag(current_tag)
   current_shape:getB2Body():SetUserData(current_tag)
   local obj_def = { tag = current_tag, tag_str = 'drawn_shape_' .. current_tag }
   RegisterObject(obj_def, obj_def.tag, obj_def.tag_str)
   current_tag = current_tag + 1

   return true
end

--- Sample OnTouchMoved for drawing-based games.  For bespoke drawing behaviour
-- clone and modify this code.
drawing.OnTouchMoved = function(x, y)
    -- Draw line segments as the touch moves
    new_pos = ccp(x, y)
    local length = ccpDistance(new_pos, last_pos);
    if length > 10 then
        drawing.AddLineToShape(current_shape, last_pos, new_pos, brush_color)
        last_pos = new_pos
    end
end

--- Sample OnTouchEnded for drawing-based games.  For bespoke drawing behaviour
-- clone and modify this code.
drawing.OnTouchEnded = function(x, y)
    -- Draw the final line segment and the end point of the line
    new_pos = ccp(x, y)
    local length = ccpDistance(new_pos, last_pos);
    if length > 5 then
        drawing.AddLineToShape(current_shape, last_pos, new_pos, brush_color)
    end
    drawing.DrawEndPoint(current_shape, new_pos, brush_color)
    last_pos = nil
    current_shape = nil
end
