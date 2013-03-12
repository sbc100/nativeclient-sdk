-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

-- Main entry points of the lua game engine.
-- Currently this file exposed 3 functions to the C++ code during
-- startup:
--  - LoadGame  (called my game_manager to load game.def)
--  - LoadLevel  (called by level_layer to load a level)
--  - LevelCount  (called by frontend so it can draw the level selction)
--
-- There are also 3 functions for which the game can define its own
-- handlers:
--  - BeginContact
--  - EndContact
--  - StartLevel

require('path')
require('util')
require('validate')

-- Make local alias of util functions so loader code can be shorter
-- and easier to read.
local Log = util.Log
local ScreenToWorld = util.ScreenToWorld
local tags = util.tags

-- The currently loaded game (set by LoadGame)
game_obj = nil

-- The currently loaded level (set by LoadLevel)
level_obj = nil

local function AddShapeToBody(body, shape, sensor)
    local fixture_def = b2FixtureDef:new_local()
    fixture_def.shape = shape
    fixture_def.density = 1.0
    fixture_def.friction = 0.5
    fixture_def.restitution = 0.3
    fixture_def.isSensor = sensor
    body:CreateFixture(fixture_def)
end

local function AddSphereToBody(body, radius, sensor)
    local shape = b2CircleShape:new_local()
    shape.m_radius = ScreenToWorld(radius)
    AddShapeToBody(body, shape, sensor)
end

--- Load game def from the given filename.  This function loads
-- the game.def file which is essentailly a dictionary and performs
-- a bit of post-processing on it.
local function LoadGameDef(filename)
    Log('loading gamedef: '..filename)
    local game = dofile(filename)
    game.root = path.dirname(filename)
    validate.ValidateGameDef(filename, game)
    Log('found ' .. #game.levels .. ' level(s)')
    game.filename = filename

    if game.script ~= nil then
        Log('loading game script: ' .. game.script)
        game.script = dofile(path.join(game.root, game.script))
    end
    return game
end

--- Create a fix pivot point between the world and the given object.
local function CreatePivot(anchor, body)
    local anchor_point = b2Vec2:new_local(ScreenToWorld(anchor.x),
                                          ScreenToWorld(anchor.y))

    -- create a new fixed body to pivot against
    local ground_def = b2BodyDef:new_local()
    ground_def.position = anchor_point
    local ground_body = level_obj.world:CreateBody(ground_def);

    -- create the pivot joint
    local joint_def = b2RevoluteJointDef:new_local()
    joint_def:Initialize(ground_body, body, anchor_point)
    local joint = level_obj.world:CreateJoint(joint_def)
end

--- Create a line between two points in the given Box2D world.
-- The brush is used to create a sequence of sprites to represent
-- the line.
local function CreateLine(brush, from, to, object)
    -- create body
    local body_def = b2BodyDef:new_local()
    body_def.position:Set(ScreenToWorld(from.x), ScreenToWorld(from.y))
    if object.dynamic == true then
        body_def.type = b2_dynamicBody
    end
    local body = level_obj.world:CreateBody(body_def)

    if object.anchor ~= nil then
        CreatePivot(object.anchor, body)
    end

    -- calculate thickness based on brush sprite size
    local brush_tex = brush:getTexture()
    local brush_size = brush_tex:getContentSizeInPixels()
    local thickness = math.max(brush_size.height/2, brush_size.width/2);

    -- calculate length and angle of line based on start and end points
    local length = ccpDistance(from, to);
    local dist_x = to.x - from.x
    local dist_y = to.y - from.y
    local angle = math.atan2(dist_y, dist_x)
    Log('adding sprite at: '..from.x..'x'..from.y)

    -- create fixture
    local center = b2Vec2:new_local(ScreenToWorld(dist_x/2),
                                    ScreenToWorld(dist_y/2))
    local shape = b2PolygonShape:new_local()
    shape:SetAsBox(ScreenToWorld(length/2), ScreenToWorld(thickness),
                   center, angle)
    local fixture_def = b2FixtureDef:new_local()
    fixture_def.shape = shape
    fixture_def.density = 1.0
    fixture_def.friction = 0.5
    fixture_def.restitution = 0.3
    body:CreateFixture(fixture_def)

    -- Now create a visible CCPhysicsSprite that the body is attached to
    local sprite = CCPhysicsSprite:createWithTexture(brush_tex)
    sprite:setB2Body(body)
    sprite:setPTMRatio(util.PTM_RATIO)

    -- And add a sequence of non-physics sprites as children of the first
    local dist = CCPointMake(dist_x, dist_y)
    local num_children = length / thickness
    local inc_x = dist_x / num_children
    local inc_y = dist_y / num_children
    local child_location = CCPointMake(thickness, thickness)
    for i = 1,num_children do
        child_location.x = child_location.x + inc_x
        child_location.y = child_location.y + inc_y

        local child_sprite = CCSprite:createWithTexture(brush_tex)
        child_sprite:setPosition(child_location)
        sprite:addChild(child_sprite)
    end

    return sprite
end

--- Create CCPoint from a lua table containing 2 elements.
-- The point is then offset according the origin.
local function PointFromLua(point)
    return CCPointMake(point[1] + game_obj.origin.x, point[2] + game_obj.origin.y)
end

--- Create a physics sprite at a fiven location with a given image
local function CreatePhysicsSprite(layer, objectdef)
    local pos = PointFromLua(objectdef.pos)
    Log('new sprite [tag=' .. objectdef.tag .. ' image=' .. objectdef.image .. ']: '..pos.x..'x'..pos.y)
    local image = game_obj.assets[objectdef.image]
    local sprite = CCPhysicsSprite:create(image)
    local body_def = b2BodyDef:new_local()
    if not objectdef.sensor then
        body_def.type = b2_dynamicBody
    end
    local body = level_obj.world:CreateBody(body_def)
    body:SetUserData(objectdef.tag)
    sprite:setB2Body(body)
    sprite:setPTMRatio(util.PTM_RATIO)
    sprite:setPosition(pos)
    AddSphereToBody(body, sprite:boundingBox().size.height/2, objectdef.sensor)
    layer:addChild(sprite, 1, objectdef.tag)
    return sprite
end

--- Load game data from a given root directory.
-- This game then becomes the currently running game.
-- @param The root directory of the game to be loaded.
function LoadGame(root_dir)
   game_root = root_dir
   game_obj = LoadGameDef(path.join(game_root, 'game.def'))
end

--- Determine the number of levels in the currently loaded game.
function LevelCount(layer)
   local icon = CCSprite:create(game_obj.assets.level_icon)
   local icon_selected = CCSprite:create(game_obj.assets.level_icon_selected)
   layer:addChild(icon, 1, util.tags.LEVEL_ICON)
   layer:addChild(icon_selected, 1, util.tags.LEVEL_ICON_SELECTED)
   return #game_obj.levels
end

--- Load the given level of the given game
-- @param layer The level to populate with game objects
-- @param level_number The level to load
function LoadLevel(layer, level_number)
    Log('loading level ' .. level_number .. ' from ' .. game_obj.filename)
    -- Get level descrition object
    assert(level_number <= #game_obj.levels and level_number > 0,
           'Invalid level number: ' .. level_number)
    local filename = path.join(game_obj.root, game_obj.levels[level_number])
    level = dofile(filename)
    validate.ValidateLevelDef(filename, game_obj, level)

    local assets = game_obj.assets
    level_obj = {}
    level_obj.layer = layer
    level_obj.world = layer:GetWorld()
    level_obj.leveldef = level

    game_obj.origin = CCDirector:sharedDirector():getVisibleOrigin()

    for _, sprite in ipairs(level.sprites) do
        CreatePhysicsSprite(layer, sprite)
        if sprite.script then
            local script = path.join(game_obj.root, sprite.script)
            Log('loading object script: ' .. script)
            sprite.script = dofile(script)
        end
    end

    -- Load brush image
    local brush = CCSpriteBatchNode:create(assets.brush_image, 500)
    layer:addChild(brush, 1, tags.BRUSH)

    -- Load fixtures
    if level.shapes then
        for _, shape in ipairs(level.shapes) do
            if shape.type == 'line' then
                local start = PointFromLua(shape.start)
                local finish = PointFromLua(shape.finish)
                if shape.anchor ~= nil then
                    shape.anchor = PointFromLua(shape.anchor)
                end
                local line = CreateLine(brush, start, finish, shape)
                brush:addChild(line, 1)
            else
                assert(false)
            end
        end
    end

    StartLevel(layer, level_number)
end

local function CallCollisionHandler(tag1, tag2, handler_name)
    local object1 = level_obj.leveldef.object_map[tag1]
    local object2 = level_obj.leveldef.object_map[tag2]

    -- only call handlers if the objects in question have tags
    -- that are known to the currently running level
    if object1 == nil or object2 == nil then
        return
    end

    -- call the individual object's collision handler, if any
    if object1.script ~= nil and object1.script[handler_name] ~= nil then
        object1.script[handler_name](object2)
    end
    if object2.script ~= nil and object2.script[handler_name] ~= nil then
        object2.script[handler_name](object1)
    end

    -- call the game's collision handler, if any
    if game_obj.script[handler_name] ~= nil then
        game_obj.script[handler_name](object1, object2)
    end
end

function BeginContact(tag1, tag2)
    CallCollisionHandler(tag1, tag2, 'BeginContact')
end

function EndContact(tag1, tag2)
    CallCollisionHandler(tag1, tag2, 'EndContact')
end

function StartLevel(layer, level_number)
    -- only call handlers if the objects in question have tags
    -- that are known to the currently running level
    if game_obj.script.StartLevel ~= nil then
        game_obj.script.StartLevel(layer, level_number)
    end
end
