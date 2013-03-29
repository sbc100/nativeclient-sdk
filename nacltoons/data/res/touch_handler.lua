-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

-- Touch handling code for game engine.
-- This module defines a singe global table touch_handler which contains
-- a single function called TouchHandler.  This functions registered
-- as the touch handler for CCLayer in which the gameplace takes place.

local util = require 'util'

local touch_handler = {}

local touch_state = {
    touchid = -1,
    reciever = nil,
    is_object = false,
}

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

local lasttap_time = 0
local lasttap_count = 1
local lasttap_location = nil

touch_handler.DOUBLE_CLICK_INTERVAL = 0.250
touch_handler.DOUBLE_CLICK_TOLERANCE = 5

local function OnTouchBegan(x, y, touchid)
    -- Do double tap detection based on taps that occur within
    -- DOUBLE_CLICK_INTERVAL of each other and with a certain
    -- distance of each other.
    local now = CCTime:getTime()
    local distance = 0
    if lasttap_location then
        distance = ccpDistance(ccp(x, y), ccp(lasttap_location[1], lasttap_location[2]))
    end
    if (now - lasttap_time > touch_handler.DOUBLE_CLICK_INTERVAL
        or distance > DOUBLE_CLICK_TOLERANCE) then
        lasttap_count = 1
    else
        lasttap_count = lasttap_count + 1
    end
    lasttap_time = now
    lasttap_location = { x, y }

    tapcount = lasttap_count
    util.Log('tap ' .. tapcount)

    local obj_def = FindTaggedBodyAt(x, y, 0x1)
    if obj_def  then
        if obj_def.script and obj_def.script.OnTouchBegan then
            if obj_def.script.OnTouchBegan(obj_def, x, y, tapcount) then
                touch_state.touchid = touchid
                touch_state.receiver = obj_def
                touch_state.is_object = true
                return true
            end
        end
    end

    -- If neither object handles the touch then ask the level
    if level_obj.script and level_obj.script.OnTouchBegan then
        if level_obj.script.OnTouchBegan(x, y, tapcount) then
            touch_state.touchid = touchid
            touch_state.receiver = level_obj
            touch_state.is_object = false
            return true
        end
    end

    -- Finally ask the game.
    if game_obj.script.OnTouchBegan then
        if game_obj.script.OnTouchBegan(x, y, tapcount) then
            touch_state.touchid = touchid
            touch_state.receiver = game_obj
            touch_state.is_object = false
            return true
        end
    end

    -- No object handle the touch.  Returning false tells cocos2dx we are not
    -- interested in this particular touch.
    return false
end

local function OnTouchMoved(x, y, touchid)
    -- ignore touch moves unless the touchid matches the one we are currently tracking
    if touchid == touch_state.touchid then
        if touch_state.receiver.script.OnTouchMoved then
            if touch_state.is_object then
                touch_state.receiver.script.OnTouchMoved(receiver, x, y)
            else
                touch_state.receiver.script.OnTouchMoved(x, y)
            end
        end
    end
end

local function OnTouchEnded(x, y, touchid)
    -- ignore touch ends unless the touchid matches the one we are currently tracking
    if touchid == touch_state.touchid then
        if touch_state.receiver.script.OnTouchEnded then
            if touch_state.is_object then
                touch_state.receiver.script.OnTouchEnded(receiver, x, y)
            else
                touch_state.receiver.script.OnTouchEnded(x, y)
            end
        end
        touch_state.touchid = -1
        touch_state.receiver = nil
        touch_state.is_object = false
    end
end

function touch_handler.TouchHandler(touch_type, x, y, touchid)
    if touch_type == 'began' then
        return OnTouchBegan(x, y, touchid)
    elseif touch_type == 'moved' then
        return OnTouchMoved(x, y, touchid)
    elseif touch_type == 'ended' then
        return OnTouchEnded(x, y, touchid)
    end
end

return touch_handler
