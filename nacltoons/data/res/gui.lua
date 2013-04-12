-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

--- GUI function for creating menus, etc..

local util = require 'util'
local gui = {
  FONT_NAME = 'Arial.ttf',
  FONT_SIZE = 48
}

function gui.CreateLabel(label_def)
  local font_name = label_def.font_name or gui.FONT_NAME
  local font_size = label_def.font_size or gui.FONT_SIZE
  local name = label_def.name or "UNNAMED"
  local value = label_def.value or -1

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
    label_def.value = label_def.value or index
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
      label:setPositionX(0 - size.width / 2 )
    end
    if menu_def.align == 'Left' then
      size = label:getContentSize()
      label:setPositionX(size.width / 2)
    end
  end

  return menu
end


return gui
