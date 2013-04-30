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
local brush_node
local brush_thickness

-- Constant for grouping physics bodies
local MAIN_CATEGORY = 0x1
local DRAWING_CATEGORY = 0x2

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

local function InitPhysicsSprite(sprite, location, dynamic)
    local body_def = b2BodyDef:new_local()
    if dynamic == true then
        body_def.type = b2_dynamicBody
    end
    local body = level_obj.world:CreateBody(body_def)
    sprite:setB2Body(body)
    sprite:setPTMRatio(util.PTM_RATIO)
    sprite:setPosition(location)
    return body
end

local function DrawBrush(parent, location, color)
    local child_sprite = CCSprite:createWithTexture(brush_tex)
    child_sprite:setPosition(location)
    child_sprite:setColor(color)
    parent:addChild(child_sprite)
end

function DrawPhysicsBrush(location, color, tag, dynamic)
    local sprite = CCPhysicsSprite:createWithTexture(brush_tex)
    local body = InitPhysicsSprite(sprite, location, dynamic)
    sprite:setColor(color)
    sprite:setTag(tag)
    body:SetUserData(tag)
    brush_node:addChild(sprite)
    return sprite
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
    local num_children = math.ceil(length / brush_step)
    local inc_x = dist_x / num_children
    local inc_y = dist_y / num_children
    local child_location = ccp(relative_start_x + brush_thickness, relative_start_y + brush_thickness)

    util.Log('Create line at: ' .. util.PointToString(from) .. ' len=' .. length .. ' num=' .. num_children)
    for i = 1,num_children do
        child_location.x = child_location.x + inc_x
        child_location.y = child_location.y + inc_y
        DrawBrush(sprite, child_location, color)
    end

    return fixture
end

--- Create a line between two points.
-- Uses a sequence of brush sprites an a single box2d rect.
local function CreateLine(from, to, objdef)
    -- create body
    util.Log("Creating line with tag " .. objdef.tag)

    if objdef.color then
        color = ccc3(objdef.color[1], objdef.color[2], objdef.color[3])
    else
        color = ccc3(255, 255, 255)
    end

    local sprite = drawing.DrawStartPoint(from, color, objdef.tag, objdef.dynamic)
    if objdef.anchor then
        CreatePivot(objdef.anchor, sprite:getB2Body())
    end
    AddLineToShape(sprite, from, to, color)
    return sprite
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
    brush_node = brush
    brush_tex = brush:getTexture()
    local brush_size = brush_tex:getContentSizeInPixels()
    brush_thickness = math.max(brush_size.height/2, brush_size.width/2)
    brush_step = brush_thickness * 1.5
end

--- Draw a shape described by a given shape def.
-- This creates physics sprites and accosiated box2d bodies for
-- the shape.
function drawing.CreateShape(shape_def)
    if shape_def.type == 'line' then
        local start = util.PointFromLua(shape_def.start)
        local finish = util.PointFromLua(shape_def.finish)
        if shape_def.anchor then
            shape_def.anchor = util.PointFromLua(shape_def.anchor)
        end
        return CreateLine(start, finish, shape_def)
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
function drawing.CreateSprite(sprite_def)
    local pos = util.PointFromLua(sprite_def.pos)
    -- util.Log('Create sprite [tag=' .. sprite_def.tag .. ' image=' .. sprite_def.image .. ']: ' ..
    --    util.PointToString(pos))
    local image = game_obj.assets[sprite_def.image]
    local sprite = CCPhysicsSprite:create(image)
    local dynamic = not sprite_def.sensor
    local body = InitPhysicsSprite(sprite, pos, dynamic)
    body:SetUserData(sprite_def.tag)

    AddSphereToBody(body, pos, sprite:boundingBox().size.height/2, sprite_def.sensor)
    return sprite
end

--- Create a single circlular point with the brush.
-- This is used to start shapes that the use draws.  The starting
-- point contains the box2d body for the shape.
function drawing.DrawStartPoint(location, color, tag, dynamic)
    -- Add visible sprite
    local sprite = DrawPhysicsBrush(location, color, tag, dynamic)

    -- Add collision info
    local fixture = AddSphereToBody(sprite:getB2Body(), location, brush_thickness, false)
    SetCategory(fixture, DRAWING_CATEGORY)

    return sprite
end

--- Create a circle composed of brush sprites backed by a single box2d
-- circle fixture.
function drawing.DrawCircle(center, radius, color, tag)
    -- Create an initial, invisble sprite at the center, to which we
    -- attach a sequence of visible child sprites

    local inner_radius = math.max(radius - brush_thickness, 1)
    local circumference = 2 * math.pi * inner_radius
    local num_sprites = math.max(circumference / brush_step, 1)
    local angle_delta = 2 * math.pi / num_sprites

    -- The firsh brush draw is the origin of the physics object.
    local start_point = ccp(center.x + inner_radius, center.y)
    local sprite = DrawPhysicsBrush(start_point, color, tag)

    local start_offset = ccp(start_point.x - center.x, start_point.y - center.y)

    local anchor = sprite:getAnchorPointInPoints()
    util.Log('drawing circle: radius=' .. math.floor(radius) .. ' sprites=' .. num_sprites)
    util.Log('drawing circle: anchor=' .. util.PointToString(anchor))
    for angle = angle_delta, 2 * math.pi, angle_delta do
        x = inner_radius * math.cos(angle)
        y = inner_radius * math.sin(angle)
        local pos = ccp(x - start_offset.x + anchor.x, y - start_offset.y + anchor.y)
        DrawBrush(sprite, pos, color)
    end

    -- Create the box2d physics body to match the sphere.
    local fixture = AddSphereToBody(sprite:getB2Body(), center, radius, false)
    SetCategory(fixture, DRAWING_CATEGORY)

    return sprite
end

function drawing.DrawEndPoint(sprite, location, color)
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
end

function drawing.AddLineToShape(sprite, from, to, color)
    fixture = AddLineToShape(sprite, from, to, color)
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
