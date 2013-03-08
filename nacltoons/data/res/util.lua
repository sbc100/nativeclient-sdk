-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

--- Utility functions and constants shared by nacltoons lua code

util = {}

util.PTM_RATIO = 32

-- Tags used when lua creates nodes. This allows the C++ side to
-- look up the nodes using these tags.
util.tags = {
    LAYER_PHYSICS = 100,
    LAYER_UI = 101,
    BALL = 1,
    GOAL = 2,
    STAR1 = 3,
    STAR2 = 4,
    STAR3 = 5,
    BRUSH = 6,
    LEVEL_ICON = 7,
    LEVEL_ICON_SELECTED = 8,
    OBJECTS_START = 256,
}

-- Convert a value from screen coordinate system to Box2D world coordinates
util.ScreenToWorld = function(value)
    return value / util.PTM_RATIO
end

--- Log messages to console
util.Log = function(...)
    print('LUA: '..string.format(...))
end
