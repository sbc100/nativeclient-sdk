-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

--- Utility functions and constants shared by nacltoons lua code

local yaml = require 'yaml'

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
function util.PointFromLua(point, absolute)
    if absolute == nil then
        absolute = true
    end
    if abolute then
        return ccp(point[1] + game_obj.origin.x, point[2] + game_obj.origin.y)
    else
        return ccp(point[1], point[2])
    end
end

--- Convert CCPoint to b2Vec.
function util.b2VecFromCocos(cocos_vec)
    return b2Vec2:new_local(util.ScreenToWorld(cocos_vec.x),
                            util.ScreenToWorld(cocos_vec.y))
end

--- Load a yaml file and return a lua table that represents the data
-- in the file.
function util.LoadYaml(filename)
    local filedata = io.open(filename, "r"):read("*all")
    return yaml.load(filedata)
end

--- Escape string for inclusion in yaml output.  Normal strings
-- that contain no special characters don't even need quoting in
-- in yaml.  This function adds quotes and escape chars as needed.
local function EscapeYamlString(str)
    -- TODO(sbc): implement this once we start needs to use special
    -- chars in our data files.
    return str
end

function util.TableToYamlOneLine(tt, ignore_keys, key_map)
    local sb = {}
    local is_array = #tt ~= 0
    ignore_keys = ignore_keys or {}
    key_map = key_map or {}

    if is_array then
        table.insert(sb, '[')
    else
        table.insert(sb, '{ ')
    end

    local first = true
    for key, value in pairs(tt) do
        if not ignore_keys[key] and type(value) ~= 'userdata' then
            if first then
                first = false
            else
                table.insert(sb, ', ')
            end
            key = key_map[key] or key
            if type(key) ~= 'number' then
                table.insert(sb, tostring(key) .. ': ')
            end
            if type(value) == 'table' then
                table.insert(sb, util.TableToYamlOneLine(value, ignore_keys, key_map))
            elseif type(value) == 'boolean' or type(value) == 'number' then
                table.insert(sb, string.format("%s", tostring(value)))
            elseif type(value) == 'string' then
                table.insert(sb, string.format("%s", EscapeYamlString(value)))
            elseif type(value) == 'userdata' then
                error('cannot serialise <userdata>')
            else
                table.insert(sb, string.format("'%s'", tostring(value)))
            end
        end
    end

    if is_array then
        table.insert(sb, ']')
    else
        table.insert(sb, ' }')
    end

    return table.concat(sb)
end

--- Serialise a lua data structure (table) to a yaml string
function util.TableToYaml(tt, ignore_keys, key_map, long_form, indent)
    indent = indent or 0
    ignore_keys = ignore_keys or {}
    key_map = key_map or {}

    -- special case for when the entire table fits on a single line
    if not long_form then
        local oneline = util.TableToYamlOneLine(tt, ignore_keys, key_map)
        if #oneline + indent < 80 then
            return oneline
        end
    end

    local sb = {}
    local is_array = true
    local next_indent = indent + 1
    local skip_first = true

    if #tt == 0 then
       is_array = false
    end

    if is_array then
       skip_first = false
       next_indent = indent + 2
       table.insert(sb, '\n')
    end

    for key, value in pairs(tt) do
        if not ignore_keys[key] and type(value) ~= 'userdata' then
            if skip_first then
                skip_first = false
            else
                table.insert(sb, string.rep(' ', indent))
            end
            if is_array then
                table.insert(sb, '- ')
            end
            key = key_map[key] or key
            if type(key) ~= 'number' then
                table.insert(sb, tostring(key))
                table.insert(sb, ':')
                if type(value) ~= 'table' then
                     table.insert(sb, ' ')
                end
            end
            if type(value) == 'table' then
                local childtable = util.TableToYaml(value, ignore_keys, key_map, long_form, next_indent)
                if string.sub(childtable, -1) ~= '\n' then
                    childtable = childtable .. '\n'
                    if not is_array then
                        childtable = ' ' .. childtable
                    end
                end
                table.insert(sb, childtable)
            elseif type(value) == 'boolean' or type(value) == 'number' then
                table.insert(sb, string.format("%s\n", tostring(value)))
            elseif type(value) == 'string' then
                table.insert(sb, string.format("%s\n", EscapeYamlString(value)))
            elseif type(value) == 'userdata' then
                error('cannot serialise <userdata>')
            else
                table.insert(sb, string.format("'%s'\n", tostring(value)))
            end
        end
    end

    return table.concat(sb)
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

return util
