-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

-- Touch handling code for game engine.
-- This module defines a singe global table touch_handler which contains
-- a single function called TouchHandler.  This functions registered
-- as the touch handler for CCLayer in which the gameplace takes place.

require 'util'

touch_handler = {}

local function ContainsTouchLocation(node, x, y)
    local pos_x = node:getPositionX()
    local pos_y = node:getPositionY()
    local s = node:getContentSize()
    util.Log('pos ' .. math.floor(pos_x) .. 'x' .. math.floor(pos_y))
    util.Log('size ' .. math.floor(s.width) .. 'x' .. math.floor(s.height))
    local touchRect = CCRectMake(-s.width / 2 + pos_x, -s.height / 2 + pos_y, s.width, s.height)
    return touchRect:containsPoint(ccp(x, y))
end

local current_touchid = -1
local current_touchobject = -1

local function OnTouchBegan(x, y, touchid)
    for tag, obj_def in pairs(level_obj.leveldef.object_map) do
        -- only consider objects that have an OnTouchBegan handler
        if obj_def.script and obj_def.script.OnTouchBegan then
            local sprite = level_obj.layer:getChildByTag(tag)
            if sprite then
                -- util.Log('Looking for tag: ' .. tag .. ' ' .. obj_def.tag_str)
                if ContainsTouchLocation(sprite, x, y) then
                    if obj_def.script.OnTouchBegan(x, y) then
                        current_touchid = touchid
                        current_touchobject = obj_def
                        return true
                    end
                end
            end
        end
    end

    if game_obj.script.OnTouchBegan and game_obj.script.OnTouchBegan(x, y) then
        current_touchid = touchid
        current_touchobject = game_obj
        return true
    end
end

local function OnTouchMoved(x, y, touchid)
    -- ignore touch moves unless the touchid matches the one we are currently tracking
    if touchid ~= current_touchid then
        return false
    end
    if current_touchobject.script.OnTouchMoved then
        return current_touchobject.script.OnTouchMoved(x, y)
    end
    return false
end

local function OnTouchEnded(x, y, touchid)
    -- ignore touch ends unless the touchid matches the one we are currently tracking
    if touchid ~= current_touchid then
        return
    end
    if current_touchobject.script.OnTouchEnded then
        current_touchobject.script.OnTouchEnded(x, y)
    end
    current_touchid = -1
    current_touchobject = nil
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
