-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

-- Touch handling code for game engine.
-- This module defines a singe global table touch_handler which contains
-- a single function called TouchHandler.  This functions registered
-- as the touch handler for CCLayer in which the gameplace takes place.

require 'util'

touch_handler = {}

local current_touchid = -1
local current_touchobject = -1

local function FindTaggedBodyAt(x, y)
    local b2pos = util.b2VecFromCocos(ccp(x, y))
    local body = level_obj.layer:FindBodyAt(b2pos)
    if body then
        local tag = body:GetUserData()
        if tag then
            return level_obj.object_map[tag]
        end
    end
end

local function OnTouchBegan(x, y, touchid)
    local obj_def = FindTaggedBodyAt(x, y)
    if obj_def  then
        -- print("Found touched object: " .. obj_def.tag)
        if obj_def.script and obj_def.script.OnTouchBegan then
            if obj_def.script.OnTouchBegan(x, y) then
                current_touchid = touchid
                current_touchobject = obj_def
                return true
            end
        end
    end

    -- If neither object handles the touch then ask the level
    if level_obj.script and level_obj.script.OnTouchBegan then
        if level_obj.script.OnTouchBegan(x, y) then
            current_touchid = touchid
            current_touchobject = level_obj
            return true
        end
    end

    -- Finally ask the game.
    if game_obj.script.OnTouchBegan then
        if game_obj.script.OnTouchBegan(x, y) then
            current_touchid = touchid
            current_touchobject = game_obj
            return true
        end
    end

    -- No object handle the touch.  Returning false tells cocos2dx we are not
    -- interested in this particular touch.
    return false
end

local function OnTouchMoved(x, y, touchid)
    -- ignore touch moves unless the touchid matches the one we are currently tracking
    if touchid == current_touchid then
        if current_touchobject.script.OnTouchMoved then
            current_touchobject.script.OnTouchMoved(x, y)
        end
    end
end

local function OnTouchEnded(x, y, touchid)
    -- ignore touch ends unless the touchid matches the one we are currently tracking
    if touchid == current_touchid then
        if current_touchobject.script.OnTouchEnded then
            current_touchobject.script.OnTouchEnded(x, y)
        end
        current_touchid = -1
        current_touchobject = nil
    end
end

touch_handler.TouchHandler = function(touch_type, x, y, touchid)
    if touch_type == 'began' then
        return OnTouchBegan(x, y, touchid)
    elseif touch_type == 'moved' then
        return OnTouchMoved(x, y, touchid)
    elseif touch_type == 'ended' then
        return OnTouchEnded(x, y, touchid)
    end
end
