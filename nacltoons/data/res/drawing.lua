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

local util = require 'util'

local drawing = {
    MODE_SELECT = 1,
    MODE_FREEHAND = 2,
    MODE_LINE = 3,
    MODE_RECT = 4,
    MODE_CIRCLE = 5,
}

drawing.mode = drawing.MODE_FREEHAND

-- Brush information (set by SetBrush)
local brush_tex
local brush_thickness

-- Constant for grouping physics bodies
local MAIN_CATEGORY = 0x1
local DRAWING_CATEGORY = 0x2

-- Constants for tagging cocos nodes
local TAG_BATCH_NODE = 0x1

-- Starting batch size for CCSpriteBatchNode.  We override
-- the default since we often have many sprites in each
-- node (each drawn element is it own batch node).
local DEFAULT_BATCH_COUNT = 100

-- Local state for default touch handlers
local current_shape = nil
local current_tag = 99 -- util.tags.TAG_DYNAMIC_START
local start_pos = nil
local last_pos = nil
local brush_color = ccc3(255, 100, 100)

-- Callbacks that are registered for drawn objects.  The game
-- can register its own callbacks here to add behavior for
-- drawn objects.
drawing.handlers = {}

--- Create a b2Vec from a lua list containing 2 elements.
-- This is used to convert point data from .def files directly
-- to the box2dx coordinate system
local function b2VecFromLua(luapoint)
    local ccpoint = util.PointFromLua(luapoint)
    local rtn = util.b2VecFromCocos(ccpoint)
    return rtn
end

local function CreateBrushBatch(parent)
    local node = CCSpriteBatchNode:createWithTexture(brush_tex, DEFAULT_BATCH_COUNT)
    assert(node)
    parent:addChild(node, 1, TAG_BATCH_NODE)
    return node
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

local function InitPhysicsNode(node, location, dynamic, tag)
    local body_def = b2BodyDef:new_local()
    if dynamic == true then
        body_def.type = b2_dynamicBody
    end
    local body = level_obj.world:CreateBody(body_def)
    node:setB2Body(body)
    node:setPTMRatio(util.PTM_RATIO)
    node:setPosition(location)
    node:setTag(tag)
    body:SetUserData(tag)
    level_obj.layer:addChild(node, 1, tag)
    return body
end

-- Create and initialise a new invisible physics node.
local function CreatePhysicsNode(location, dynamic, tag)
    local node = CCPhysicsNode:create()
    InitPhysicsNode(node, location, dynamic, tag)
    return node
end

local function DrawBrush(parent, location, color)
    local child_sprite = CCSprite:createWithTexture(brush_tex)
    child_sprite:setPosition(location)
    child_sprite:setColor(color)
    parent:addChild(child_sprite)
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
local function AddLineToShape(node, from, to, color, absolute)
    -- calculate length and angle of line based on start and end points
    local body = node:getB2Body()
    local length = ccpDistance(from, to);
    local dist_x = to.x - from.x
    local dist_y = to.y - from.y

    -- create fixture
    local rel_start = from
    if absolute then
       rel_start = node:convertToNodeSpace(from)
    end
    local center = b2Vec2:new_local(util.ScreenToWorld(rel_start.x + dist_x/2),
                                    util.ScreenToWorld(rel_start.y + dist_y/2))
    local shape = b2PolygonShape:new_local()
    local angle = math.atan2(dist_y, dist_x)
    shape:SetAsBox(util.ScreenToWorld(length/2), util.ScreenToWorld(brush_thickness),
                   center, angle)
    local fixture = AddShapeToBody(body, shape, false)

    -- Create sequence of sprite nodes as children
    local dist = CCPointMake(dist_x, dist_y)
    local num_children = math.ceil(length / brush_step)
    local inc_x = dist_x / num_children
    local inc_y = dist_y / num_children
    local child_location = rel_start

    util.Log('Create line at: rel=' .. util.PointToString(rel_start) .. ' len=' .. length .. ' num=' .. num_children)

    local batch_node = node:getChildByTag(TAG_BATCH_NODE)
    assert(batch_node)
    for i = 1,num_children do
        child_location.x = child_location.x + inc_x
        child_location.y = child_location.y + inc_y
        DrawBrush(batch_node, child_location, color)
    end

    return fixture
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
function drawing.SetBrush(brush)
    -- calculate thickness based on brush sprite size
    brush_tex = brush:getTexture()
    local brush_size = brush_tex:getContentSizeInPixels()
    brush_thickness = math.max(brush_size.height/2, brush_size.width/2)
    brush_step = brush_thickness * 1.5
end

--- Create a physics sprite at a given location with a given image
local function AddSpriteToShape(node, sprite_def, absolute)
    local pos = util.PointFromLua(sprite_def.pos, absolute)
    util.Log('Create sprite [tag=' .. sprite_def.tag .. ' image=' .. sprite_def.image .. ' absolute=' .. tostring(absolute) .. ']: ' ..
        util.PointToString(pos))
    local image = game_obj.assets[sprite_def.image]
    local sprite = CCSprite:create(image)
    local rel_pos
    local world_pos
    if absolute then
       rel_pos = node:convertToNodeSpace(pos)
       world_pos = pos
    else
       rel_pos = pos
       world_pos = node:convertToWorldSpace(pos)
    end
    sprite:setPosition(rel_pos)
    node:addChild(sprite)
    AddSphereToBody(node:getB2Body(), world_pos, sprite:boundingBox().size.height/2, sprite_def.sensor)
    return sprite
end

local function AddChildShape(shape, child_def, absolute)
    if child_def.color then
        color = ccc3(child_def.color[1], child_def.color[2], child_def.color[3])
    else
        color = ccc3(255, 255, 255)
    end

    if child_def.type == 'line' then
        local start = util.PointFromLua(child_def.start, absolute)
        local finish = util.PointFromLua(child_def.finish, absolute)
        AddLineToShape(shape, start, finish, color, absolute)
    elseif child_def.type == 'image' then
        AddSpriteToShape(shape, child_def, absolute)
    else
        assert(false, 'invalid shape type: ' .. shape_def.type)
    end
end

--- Draw a shape described by a given shape def.
-- This creates physics sprites and accosiated box2d bodies for
-- the shape.
function drawing.CreateShape(shape_def)
    local shape = nil

    if shape_def.type == 'compound' then
        local pos = util.PointFromLua(shape_def.pos)
        shape = CreatePhysicsNode(pos, shape_def.dynamic, shape_def.tag)
        CreateBrushBatch(shape)
        if shape_def.children then
            for _, child_def in ipairs(shape_def.children) do
                child_def.tag = shape_def.tag
                child = AddChildShape(shape, child_def, false)
            end
        end
    elseif shape_def.type == 'line' then
        local pos = util.PointFromLua(shape_def.start)
        shape = CreatePhysicsNode(pos, shape_def.dynamic, shape_def.tag)
        CreateBrushBatch(shape)
        AddChildShape(shape, shape_def, true)
    elseif shape_def.type == 'edge' then
        local body_def = b2BodyDef:new_local()
        local body = level_obj.world:CreateBody(body_def)
        local b2shape = b2EdgeShape:new_local()
        local start = b2VecFromLua(shape_def.start)
        local finish = b2VecFromLua(shape_def.finish)
        util.Log('Create edge from: ' .. util.VecToString(start) .. ' to: ' .. util.VecToString(finish))
        b2shape:Set(start, finish)
        body:CreateFixture(b2shape, 0)
        return
    elseif shape_def.type == 'image' then
        local pos = util.PointFromLua(shape_def.pos)
        shape = CreatePhysicsNode(pos, shape_def.dynamic, shape_def.tag)
        AddChildShape(shape, shape_def, true)
    else
        assert(false, 'invalid shape type: ' .. shape_def.type)
    end

    if shape_def.anchor then
        local body = shape:getB2Body()
        local anchor = util.PointFromLua(shape_def.anchor)
        CreatePivot(anchor, body)
    end

    return shape
end

--- Create a single circlular point with the brush.
-- This is used to start shapes that the user draws.  The returned
-- node is the an invisible node that acts as the physics objects.
-- Sprite nodes are then attached to this as the user draws.
function drawing.DrawStartPoint(location, color, tag, dynamic)
    -- Add invisibe physics node
    local node = CreatePhysicsNode(location, dynamic, tag)
    CreateBrushBatch(node)

    -- Add visible sprite
    local sprite = CCSprite:createWithTexture(brush_tex)
    sprite:setColor(color)
    node:addChild(sprite)

    -- Add collision info
    local fixture = AddSphereToBody(node:getB2Body(), location, brush_thickness, false)
    SetCategory(fixture, DRAWING_CATEGORY)

    return node
end

--- Create a circle composed of brush sprites backed by a single box2d
-- circle fixture.
function drawing.DrawCircle(center, radius, color, tag)
    -- Create the initial (invisible) node at the center
    -- and then attach a sequence of visible child sprites
    local node = CreatePhysicsNode(center, true, tag)
    local batch_node = CreateBrushBatch(node)

    local inner_radius = math.max(radius - brush_thickness, 1)
    local circumference = 2 * math.pi * inner_radius
    local num_sprites = math.max(circumference / brush_step, 1)
    local angle_delta = 2 * math.pi / num_sprites

    util.Log('drawing circle: radius=' .. math.floor(radius) .. ' sprites=' .. math.floor(num_sprites))
    for angle = 0, 2 * math.pi, angle_delta do
        x = inner_radius * math.cos(angle)
        y = inner_radius * math.sin(angle)
        DrawBrush(batch_node, ccp(x, y), color)
    end

    -- Create the box2d physics body to match the sphere.
    local fixture = AddSphereToBody(node:getB2Body(), center, radius, false)
    SetCategory(fixture, DRAWING_CATEGORY)
    return node
end

function drawing.DrawEndPoint(node, location, color)
    -- Add visible sprite
    local child_sprite = CCSprite:createWithTexture(brush_tex)
    child_sprite:setPosition(node:convertToNodeSpace(location))
    child_sprite:setColor(color)
    node:addChild(child_sprite)

    -- Add collision info
    local body = node:getB2Body()
    local fixture = AddSphereToBody(body, location, brush_thickness, false)
    SetCategory(fixture, DRAWING_CATEGORY)
end

function drawing.AddLineToShape(sprite, from, to, color)
    fixture = AddLineToShape(sprite, from, to, color, true)
    SetCategory(fixture, DRAWING_CATEGORY)
end

function drawing.IsDrawing()
   return current_shape ~= nil
end

--- Sample OnTouchBegan for drawing-based games.  For bespoke drawing behaviour
-- clone and modify this code.
function drawing.OnTouchBegan(x, y)
    --- ignore touches if we are already drawing something
    if drawing.IsDrawing() then
        return false
    end

    if drawing.mode == drawing.MODE_SELECT then
        return false
    end

    start_pos = ccp(x, y)
    last_pos = start_pos

    -- New shape def
    local shape = {
        tag = current_tag,
        tag_str = 'drawn_shape_' .. current_tag,
        script = drawing.handlers,
    }

    if drawing.mode == drawing.MODE_FREEHAND or drawing.mode == drawing.MODE_LINE then
        -- create initial sphere to represent start of shape
        shape.node = drawing.DrawStartPoint(start_pos, brush_color, current_tag)
    elseif drawing.mode == drawing.MODE_CIRCLE then
        shape.node = drawing.DrawCircle(start_pos, 1, brush_color, current_tag)
    else
        error('invalid drawing mode: ' .. tostring(drawing.mode))
    end

    -- Register the new shape
    RegisterObject(shape, shape.tag, shape.tag_str)

    current_shape = shape
    current_tag = current_tag + 1
    return true
end

function drawing.DestroySprite(sprite)
   -- Work around crash bug!
   sprite:setPosition(sprite:getPositionX(), sprite:getPositionY())
   local body = sprite:getB2Body()
   sprite:removeFromParentAndCleanup(true)
   body:GetWorld():DestroyBody(body)
end

--- Sample OnTouchMoved for drawing-based games.  For bespoke drawing behaviour
-- clone and modify this code.
function drawing.OnTouchMoved(x, y)
    new_pos = ccp(x, y)
    if drawing.mode == drawing.MODE_FREEHAND then
        -- Draw line segments as the touch moves
        local length = ccpDistance(new_pos, last_pos);
        if length > brush_thickness * 2 then
            drawing.AddLineToShape(current_shape.node, last_pos, new_pos, brush_color)
            last_pos = new_pos
        end
    elseif drawing.mode == drawing.MODE_LINE then
        local tag = current_shape.node:getTag()
        drawing.DestroySprite(current_shape.node)

        current_shape.node = drawing.DrawStartPoint(start_pos, brush_color, tag)
        drawing.AddLineToShape(current_shape.node, start_pos, new_pos, brush_color)
    elseif drawing.mode == drawing.MODE_CIRCLE then
        local tag = current_shape.node:getTag()
        drawing.DestroySprite(current_shape.node)
        radius = ccpDistance(start_pos, new_pos)
        current_shape.node = drawing.DrawCircle(start_pos, radius, brush_color, tag)
    else
        error('invalid drawing mode: ' .. tostring(drawing.mode))
    end
end

--- Sample OnTouchEnded for drawing-based games.  For bespoke drawing behaviour
-- clone and modify this code.
function drawing.OnTouchEnded(x, y)
    -- Draw the final line segment and the end point of the line

    if drawing.mode == drawing.MODE_FREEHAND then
        new_pos = ccp(x, y)
        local length = ccpDistance(new_pos, last_pos);
        if length > brush_thickness then
            drawing.AddLineToShape(current_shape.node, last_pos, new_pos, brush_color)
        end
        drawing.DrawEndPoint(current_shape.node, new_pos, brush_color)
    elseif drawing.mode == drawing.MODE_CIRCLE or drawing.mode == drawing.MODE_LINE then
        --
    else
        error('invalid drawing mode: ' .. tostring(drawing.mode))
    end

    MakeBodyDynamic(current_shape.node:getB2Body())

    local rtn = current_shape
    last_pos = nil
    start_pos = nil
    current_shape = nil
    return rtn
end

return drawing
