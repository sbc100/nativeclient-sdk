-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

-- Example/default game logic.  This code displays the level selection
-- menu and triggers the loading of individual levels.


local util = require 'util'
local gui = require 'gui'
local editor = require 'editor'

local handlers = {}


-- Horizontally center a node within a layer at a given height.
local function Center(node, height)
    local visible_size = CCDirector:sharedDirector():getVisibleSize()
    local node_size = node:getContentSize()
    -- A negative height means an offet from the top of the visible area
    if height < 0 then
        height = height + visible_size.height
    end
    local xpos = game_obj.origin.x + visible_size.width / 2
    local ypos = game_obj.origin.y + height - node_size.height / 2
    node:setPosition(CCPointMake(xpos, ypos))
end

local function ElasticMove(node, start_pos, end_pos, duration, period)
    node:setPosition(start_pos)
    local move_to = CCMoveTo:create(duration, end_pos)
    node:runAction(CCEaseElasticOut:create(move_to, period))
end

--- Local function for creating the level selection menu.
local function CreateLevelMenu(layer)
    local label = gui.CreateLabel({name="Select Level"})
    layer:addChild(label)
    -- Center label 100 pixels from the top of the sceen
    Center(label, -100)
    local start_pos = ccp(label:getPositionX(), label:getPositionY() + 100)
    ElasticMove(label, start_pos, ccp(label:getPosition()), 1.0, 0.3)

    -- Create the level selection menu
    local menu = CCMenu:create()

    local icon = CCSprite:create(game_obj.assets.level_icon)
    local icon_size = icon:getContentSize()
    local label_pos = CCPointMake(icon_size.width/2, icon_size.height/2)

    local function LevelSelected(tag)
        local level_number = tag
        util.Log('LevelSelected: ' .. level_number)
        GameManager:sharedManager():LoadLevel(level_number)
    end

    -- For each level create a menu item and a textual label
    for i=1,#game_obj.levels do
        local label_string = string.format("%d", i)
        local item = CCMenuItemImage:create(game_obj.assets.level_icon,
                                            game_obj.assets.level_icon_selected)
        menu:addChild(item)
        item:setTag(i)
        item:registerScriptTapHandler(LevelSelected)

        local label = gui.CreateLabel({name=label_string})
        label:setPosition(label_pos)
        item:addChild(label)
    end
    layer:addChild(menu)
    gui.GridLayout(menu, 2, CCPointMake(20, 20))

    -- Slide menu in from botton
    local end_pos = ccp(game_obj.origin.x, game_obj.origin.y)
    local start_pos = ccp(end_pos.x, end_pos.y - 600)
    ElasticMove(menu, start_pos, end_pos, 1.0, 0.7)
end


local function MainMenuCallback(value)
    local scene = CCScene:create()
    local layer = CCLayerColor:create(background_color)
    local director = CCDirector:sharedDirector()

    util.Log("Got to MainMenuCallback with " .. value)

    if value == 1 then
        scene:addChild(layer)
        CreateLevelMenu(layer)
        director:replaceScene(scene)
        game_obj.game_mode = 'play'
    elseif value == 2 then
        scene:addChild(layer)
        CreateLevelMenu(layer)
        director:replaceScene(scene)
        game_obj.script = editor
        game_obj.game_mode = 'edit'
    end
end

--- Game behaviour callback.   This function is called when the game
-- first starts and in charge of creating the first scene.
function handlers.StartGame()
    util.Log('StartGame')
    local director = CCDirector:sharedDirector()

    background_color = ccc4(0, 0xD8, 0x8F, 0xD8)

    -- Create a new scene and layer
    local scene = CCScene:create()
    local layer = CCLayerColor:create(background_color)
    scene:addChild(layer)

    local menu_def = {
        items = {
            {
              name = 'Start Game',
              callback = MainMenuCallback,
              value = 1
            },
            {
              name = 'Edit Game',
              callback = MainMenuCallback,
              value = 2
            },
        },
    }
    local menu = gui.CreateMenu(menu_def)
    layer:addChild(menu)

    director:runWithScene(scene)
end

return handlers
