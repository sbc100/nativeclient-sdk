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
    level_obj.layer = layer
    level_obj.world = layer:GetWorld()

    local assets = game_obj.assets

    -- Load brush image
    local brush = CCSpriteBatchNode:create(assets.brush_image, 500)
    layer:addChild(brush, 1, tags.BRUSH)
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
    for _, sprite_def in ipairs(level.sprites) do
        local sprite = drawing.DrawSprite(sprite_def)
        layer:addChild(sprite, 1, sprite_def.tag)
        if sprite_def.script then
            local script = path.join(game_obj.root, sprite_def.script)
            Log('loading object script: ' .. script)
            sprite_def.script = dofile(script)
        end
    end

    -- Load shapes
    if level.shapes then
        for _, shape in ipairs(level.shapes) do
            drawing.DrawShape(shape)
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
