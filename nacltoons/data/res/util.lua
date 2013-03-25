-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

--- Utility functions and constants shared by nacltoons lua code

util = {}

util.PTM_RATIO = 32

-- Tags used used by lua and C++ both.
-- look up the nodes using these tags.
util.tags = {
    BRUSH = 50,
    LAYER_PHYSICS = 100,
    LAYER_UI = 101,
}

-- Convert a value from screen coordinate system to Box2D world coordinates
util.ScreenToWorld = function(value)
    return value / util.PTM_RATIO
end

--- Log messages to console
util.Log = function(...)
    print('LUA: '..string.format(...))
end

--- Create CCPoint from a lua table containing 2 elements.
-- This is used to convert point data from .def files into
-- the cocos2dx coordinate space.
util.PointFromLua = function(point)
    return CCPointMake(point[1] + game_obj.origin.x, point[2] + game_obj.origin.y)
end
