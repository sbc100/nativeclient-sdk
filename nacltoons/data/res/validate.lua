-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

-- Functions for validating game data files (.def files).
-- This module provides a single global called 'validate' which
-- contains two function:
--   ValidateGameDef
--   ValidateLevelDef
--
-- It is possible to run this code a game.def file from the command line:
-- $ LUA_PATH="data/res/path.lua" lua ./data/res/validate.lua data/res/sample_game/game.def

local path = require 'path'
local util = require 'util'

local function Error(filename, message)
    assert(false, "Error in '" .. filename .. "':" ..  message)
end

--- Return true if a list contains a given element, false otherwise
local function ListContains(list, element)
  for _, value in ipairs(list) do
      if value == element then
          return true
      end
  end
  return false
end

--- Check that all the keys in the given object are in the list
-- of valid keys.
local function CheckValidKeys(filename, object, valid_keys)
    for key, _ in pairs(object) do
        if not ListContains(valid_keys, key) then
            return Error(filename, 'invalid key: ' .. key)
        end
    end
end

local function CheckRequiredKeys(filename, object, required_keys, name)
    for _, required in ipairs(required_keys) do
        if object[required] == nil then
            Error(filename, 'missing required key in ' .. name .. ' : ' .. required)
        end
    end
end

validate = {}

--- Validate a game.def file.
-- @param filename the filename the def file was read from (for error reporting)
-- @param the game.def lua table (result of dofile())
validate.ValidateGameDef = function(filename, gamedef)
    local function Err(message)
        Error(filename, message)
    end

    if type(gamedef) ~= 'table' then
        return Err('file does not evaluate to an object of type table')
    end

    CheckValidKeys(filename, gamedef, { 'assets', 'script', 'levels', 'root' })
    CheckRequiredKeys(filename, gamedef.assets, { 'level_icon', 'level_icon_selected' }, 'asset list')

    for asset_name, asset_file in pairs(gamedef.assets) do
        local fullname = path.join(gamedef.root, asset_file)
        local f = io.open(fullname, 'r')
        if f ~= nil then
            io.close(f)
        else
            Err('asset does not exist: ' .. gamedef.asset_name)
        end
        gamedef.assets[asset_name] = fullname
    end
end

--- Validate a level.def file.
-- @param filename the filename the def file was read from (for error reporting)
-- @param the level.def lua table (result of dofile())
validate.ValidateLevelDef = function(filename, gamedef, leveldef)
    local function Err(message)
        Error(filename, message)
    end

    if type(leveldef) ~= 'table' then
        return Err("file does not evaluate to an object of type 'table'")
    end

    CheckValidKeys(filename, leveldef, { 'num_stars', 'shapes', 'sprites', 'script' })

    if leveldef.shapes then
        local valid_keys = { 'start', 'finish', 'color', 'type', 'anchor', 'tag', 'dynamic' }
        local valid_types = { 'line', 'edge' }
        local required_keys = { 'type' }
        for _, shape in pairs(leveldef.shapes) do
            CheckValidKeys(filename, shape, valid_keys)
            CheckRequiredKeys(filename, shape, required_keys, 'shape')
            if not ListContains(valid_types, shape.type) then
                Err('invalid shape type: ' .. shape.type)
            end
        end
    end

    if leveldef.sprites then
        local valid_keys = { 'pos', 'tag', 'image', 'script', 'sensor' }
        local required_keys = { 'pos', 'image' }
        for _, sprite in pairs(leveldef.sprites) do
            CheckValidKeys(filename, sprite, valid_keys)
            CheckRequiredKeys(filename, sprite, required_keys, 'sprite')
            if gamedef.assets[sprite.image] == nil then
                Err('invalid image asset: ' .. sprite.image)
            end
        end
    end

end

if arg and #arg >= 1 then
   -- When run from the command line run validation on passed in game.def file.
   local filename = arg[1]
   local gamedef = dofile(filename)
   gamedef.root = path.dirname(filename)
   validate.ValidateGameDef(filename, gamedef)

   -- Now validate all the levels in the game.
   for _, level in ipairs(gamedef.levels) do
       filename = path.join(gamedef.root, level)
       level = dofile(filename)
       validate.ValidateLevelDef(filename, gamedef, level)
   end
end

return validate
