-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

--- Utility functions and constants shared by nacltoons lua code

local util = {}

util.PTM_RATIO = 32

util.tags = {
    TAG_DYNAMIC_START = 0xff,
}

-- Convert a value from screen coordinate system to Box2D world coordinates
function util.ScreenToWorld(value)
    return value / util.PTM_RATIO
end

--- Log messages to console
function util.Log(...)
    print('LUA: ' .. string.format(...))
end

function util.PointToString(point)
    return string.format('%dx%d', point.x, point.y)
end

--- Create CCPoint from a lua table containing 2 elements.
-- This is used to convert point data from .def files into
-- the cocos2dx coordinate space.
function util.PointFromLua(point)
    return CCPointMake(point[1] + game_obj.origin.x, point[2] + game_obj.origin.y)
end

--- Convert CCPoint to b2Vec.
function util.b2VecFromCocos(cocos_vec)
    return b2Vec2:new_local(util.ScreenToWorld(cocos_vec.x),
                            util.ScreenToWorld(cocos_vec.y))
end

--- Serialise a lua table by converting it to a string in the standard lua
-- table notation.
function util.TableToString(tt, ignore_keys, key_map, indent)
    indent = indent or 0
    ignore_keys = ignore_keys or {}
    key_map = key_map or {}

    local sb = {}
    indent = indent + 2
    table.insert(sb, '{\n')
    for key, value in pairs(tt) do
        if ignore_keys[key] == nil and type(value) ~= 'userdata' then
            table.insert(sb, string.rep(' ', indent))
            key = key_map[key] or key
            if type(key) ~= 'number' then
                table.insert(sb, tostring(key))
                table.insert(sb, ' = ')
            end
            if type(value) == 'table' then
                table.insert(sb, util.TableToString(value, ignore_keys, key_map, indent))
            elseif type(value) == 'boolean' or type(value) == 'number' then
                table.insert(sb, string.format("%s,\n", tostring(value)))
            else
                table.insert(sb, string.format("'%s',\n", tostring(value)))
            end
        end
    end

    table.insert(sb, string.rep (' ', indent - 2))
    table.insert(sb, '}\n')
    return table.concat(sb)

end

if arg then
    print(util.TableToString({mylist = { 1, 2, 3 }, foo = 'bar', hello = 123}))
end

return util
