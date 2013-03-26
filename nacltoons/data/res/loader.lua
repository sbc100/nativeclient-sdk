-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

-- Main entry points of the lua game engine.
-- Currently this file exposed 3 functions to the C++ code during
-- startup:
--  - LoadGame  (called my game_manager to load game.def)
--  - LoadLevel  (called by level_layer to load a level)
--
-- There are also 3 functions for which the game can define its own
-- handlers:
--  - OnContactBegan
--  - OnContactEnded
--  - StartLevel

require 'drawing'
require 'path'
require 'touch_handler'
require 'util'
require 'validate'

-- Make local alias of util functions so loader code can be shorter
-- and easier to read.
local Log = util.Log
local tags = util.tags

-- The currently loaded game (set by LoadGame)
game_obj = nil

-- The currently loaded level (set by LoadLevel)
level_obj = nil

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

    if game.script then
        Log('loading game script: ' .. game.script)
        game.script = dofile(path.join(game.root, game.script))
    end

    return game
end

function RegisterObject(object, tag, tag_str)
    level_obj.tag_list[tag] = tag_str
    assert(level_obj.object_map[tag] == nil, 'object_map already contains ' .. tag)
    level_obj.object_map[tag] = object
    -- If a tag_str is given then register it in the string -> int mapping
    if tag_str then
        assert(level_obj.tag_map[tag_str] == nil, 'duplicate object tag: ' .. tag_str)
        level_obj.tag_map[tag_str] = tag
    end
    if not tag_str then tag_str = '' end
    Log('object registered: ' .. tag .. " = '" .. tag_str .. "'")
end

local function RegisterObjectDef(object)
    if object.tag then
        object.tag_str = object.tag
        object.tag = #level_obj.tag_list + 1
        RegisterObject(object, object.tag, object.tag_str)
    else
        object.tag = 0
        object.tag_str = ''
    end
end

--- Load game data from a given root directory.
-- This game then becomes the currently running game.
-- @param The root directory of the game to be loaded.
function LoadGame(root_dir)
   game_root = root_dir
   game_obj = LoadGameDef(path.join(game_root, 'game.def'))
   game_obj.origin = CCDirector:sharedDirector():getVisibleOrigin()
   local default_game

   if not game_obj.script or not game_obj.script.StartGame then
       default_game = dofile('default_game.lua')
   end

   if not game_obj.script then
       game_obj.script = default_game
   end

   if game_obj.script.StartGame then
       game_obj.script.StartGame()
   else
       default_game.StartGame()
   end

   if game_obj.assets.music then
       game_obj.assets.music = CCFileUtils:sharedFileUtils():fullPathForFilename(game_obj.assets.music)
       SimpleAudioEngine:sharedEngine():preloadBackgroundMusic(game_obj.assets.music)
   end
end

local function LevelInit()
    -- level_obj.tag_map maps string tags to integer tags
    -- level_obj.tag_list is simply a list of string tags
    -- level_obj.object_map maps tags to object defs
    level_obj.tag_map = {}
    level_obj.tag_list = {}
    level_obj.object_map = {}
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
    level_obj = dofile(filename)

    validate.ValidateLevelDef(filename, game_obj, level_obj)

    LevelInit()
    level_obj.layer = layer
    level_obj.world = layer:GetWorld()

    local assets = game_obj.assets

    -- Load brush image
    local brush = CCSpriteBatchNode:create(assets.brush_image, 500)
    layer:addChild(brush, 1)
    drawing.SetBrush(brush)

    -- Start music playback
    if game_obj.assets.music then
        SimpleAudioEngine:sharedEngine():playBackgroundMusic(game_obj.assets.music, true)
    end

    -- Load background image
    if game_obj.assets.background_image then
        local winsize = CCDirector:sharedDirector():getWinSize()
        local sprite = CCSprite:create(game_obj.assets.background_image)
        sprite:setPosition(ccp(winsize.width/2, winsize.height/2))
        layer:addChild(sprite)
    end

    -- Load sprites
    for _, sprite_def in ipairs(level_obj.sprites) do
        RegisterObjectDef(sprite_def)
        local sprite = drawing.CreateSprite(sprite_def)
        layer:addChild(sprite, 1, sprite_def.tag)
        if sprite_def.script then
            local script = path.join(game_obj.root, sprite_def.script)
            Log('loading object script: ' .. script)
            sprite_def.script = dofile(script)
        end
    end

    -- Load shapes
    if level_obj.shapes then
        for _, shape_def in ipairs(level_obj.shapes) do
            RegisterObjectDef(shape_def)
            drawing.CreateShape(shape_def)
        end
    end

    -- Load custom level script
    if level_obj.script then
        Log('loading level script: ' .. level_obj.script)
        level_obj.script = dofile(path.join(game_obj.root, level_obj.script))
    end

    layer:registerScriptTouchHandler(touch_handler.TouchHandler)
    StartLevel(level_number)
end

local function CallCollisionHandler(tag1, tag2, handler_name)
    local object1 = level_obj.object_map[tag1]
    local object2 = level_obj.object_map[tag2]

    -- only call handlers if the objects in question have tags
    -- that are known to the currently running level
    if object1 == nil or object2 == nil then
        return
    end

    -- call the individual object's collision handler, if any
    if object1.script and object1.script[handler_name] then
        object1.script[handler_name](object2)
    end
    if object2.script and object2.script[handler_name] then
        object2.script[handler_name](object1)
    end

    -- call the game's collision handler, if any
    if game_obj.script[handler_name] then
        game_obj.script[handler_name](object1, object2)
    end
end

function OnContactBegan(tag1, tag2)
    CallCollisionHandler(tag1, tag2, 'OnContactBegan')
end

function OnContactEnded(tag1, tag2)
    CallCollisionHandler(tag1, tag2, 'OnContactEnded')
end

function StartLevel(level_number)
    -- only call handlers if the objects in question have tags
    -- that are known to the currently running level
    if game_obj.script.StartLevel then
        game_obj.script.StartLevel(level_number)
    end
end
