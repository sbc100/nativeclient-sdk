-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

--- GUI function for creating menus, etc..

local util = require 'util'
local gui = {
    FONT_NAME = 'Arial.ttf',
    FONT_SIZE = 48
}

--- Layout menu if a grid.
-- @param the menu object whose children are the menu items.
-- @param max_cols the max number of items to include in each row.
-- @param padding CCSize to use for padding item.
function gui.GridLayout(menu, max_cols, padding)
    local items = menu:getChildren()

    local item0 = tolua.cast(items:objectAtIndex(0), "CCNode")
    local item_size = item0:getContentSize()

    local w = item_size.width
    local h = item_size.height

    local num_rows = (items:count() + max_cols - 1) / max_cols
    -- Totol menu height, including padding between rows.
    local menu_height = num_rows * (h + padding.y) - padding.y
    local startY = (600 - menu_height) / 2

    local item_index = 0
    for i=0,num_rows-1 do
        local num_cols = math.min(max_cols, items:count() - (i * max_cols))

        local row_width = num_cols * w + padding.x * (num_cols - 1)
        local x = (800 - row_width)/2
        local y = startY + (num_rows - i - 1) * (padding.y + h)

        for i=0,num_cols-1 do
            local item = items:objectAtIndex(item_index)
            item:setAnchorPoint(CCPointMake(0,0))
            item:setPosition(ccp(x, y))
            util.Log("Setting pos of menu item %d: %.fx%.f", item_index, x, y)
            x = x + padding.x + w
            item_index = item_index + 1
        end
    end

    assert(item_index == items:count())
end

function gui.CreateLabel(label_def)
    local font_name = label_def.font_name or gui.FONT_NAME
    local font_size = label_def.font_size or gui.FONT_SIZE
    local name = label_def.name or "UNNAMED"
    local value = label_def.value or name

    local label = CCLabelTTF:create(name, font_name, font_size)
    local item = CCMenuItemLabel:create(label)

    if label_def.callback then
        local callback = function()
            util.Log("Select " .. name .. " = " .. value)
            label_def.callback(value)
        end
        item:registerScriptTapHandler(callback)
    end

    util.Log("Create Label: " .. name .. " = " .. value)
    return item
end

function gui.CreateMenu(menu_def, parent)
    local menu = CCMenu:create()
    local align = menu_def.align or 'Center'

    if menu_def.pos then
        menu:setPosition(ccp(menu_def.pos[1], menu_def.pos[2]))
        util.Log("Set Position " .. menu_def.pos[1] .. ", " ..  menu_def.pos[2])
    end

    if menu_def.size then
        menu:setContentSize(ccp(menu_def.size[1], menu_def.size[2]))
    end

    local children = {}
    for index, label_def in ipairs(menu_def.items) do
        label_def.callback = label_def.callback or menu_def.callback
        label_def.font_size = label_def.font_size or menu_def.font_size
        label_def.font_name = label_def.font_name or menu_def.font_name

        label = gui.CreateLabel(label_def)
        menu:addChild(label)
        children[index] = label
    end

    menu:alignItemsVertically()

    for index, label in ipairs(children) do
        if menu_def.align == 'Right' then
            size = label:getContentSize()
            label:setPositionX(0 - size.width / 2)
        end
        if menu_def.align == 'Left' then
            size = label:getContentSize()
            label:setPositionX(size.width / 2)
        end
    end

    return menu
end


return gui
