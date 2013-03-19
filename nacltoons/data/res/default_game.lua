-- Copyright (c) 2013 The Chromium Authors. All rights reserved.
-- Use of this source code is governed by a BSD-style license that can be
-- found in the LICENSE file.

-- Example/default game logic.  This code displays the level selection
-- menu and triggers the loading of individual levels.

local FONT_NAME = 'Arial.ttf'
local FONT_SIZE = 48

local scripts = {}

--- Local function which lays out the level selection menu in a grid
-- @param the menu object who's children are the menu items.
-- @param max_cols the max number of items to include in each row.
-- @param padding CCSize to use for padding item.
local function LayoutMenu(menu, max_cols, padding)
    local items = menu:getChildren()

    local item0 = tolua.cast(items:objectAtIndex(0), "CCNode")
    local item_size = item0:getContentSize()

    local w = item_size.width
    local h = item_size.height

    local num_rows = (items:count() + max_cols - 1) / max_cols
    local menu_height = num_rows * (h + padding.y)
    local startY = (600 - menu_height) / 2

    local item_index = 0
    for i=0,num_rows-1 do
        local num_cols = math.min(max_cols, items:count() - (i * max_cols))
        util.Log(num_cols)

        local row_width = num_cols * w + padding.x * (num_cols - 1)
        local startX = (800 - row_width)/2
        local y = startY + (num_rows - i - 1) * (padding.y + h)

        for i=0,num_cols-1 do
            local item = items:objectAtIndex(item_index)
            item:setAnchorPoint(CCPointMake(0,0))
            item:setPosition(ccp(startX, y))
            util.Log("Setting pos of menu item %d: %.fx%.f", item_index, startX, y)
            startX = startX + padding.x + w
            item_index = item_index + 1
        end
    end

    assert(item_index == items:count())
end

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
    local label = CCLabelTTF:create("Select Level", FONT_NAME, FONT_SIZE)
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

        local label = CCLabelTTF:create(label_string, FONT_NAME, FONT_SIZE)
        label:setPosition(label_pos)
        item:addChild(label)
    end
    layer:addChild(menu)
    LayoutMenu(menu, 2, CCPointMake(20, 20))

    -- Slide menu in from botton
    local end_pos = ccp(game_obj.origin.x, game_obj.origin.y)
    local start_pos = ccp(end_pos.x, end_pos.y - 600)
    ElasticMove(menu, start_pos, end_pos, 1.0, 0.7)
end

--- Game behaviour callback.   This function is called when the game
-- first starts and in charge of creating the first scene.
scripts.StartGame = function()
    util.Log('StartGame')
    local director = CCDirector:sharedDirector()

    background_color = ccc4(0, 0xD8, 0x8F, 0xD8)

    -- Create a new scene and layer
    local scene = CCScene:create()
    local layer = CCLayerColor:create(background_color)
    scene:addChild(layer)

    local start_label = CCLabelTTF:create("Start Game", FONT_NAME, FONT_SIZE)

    local function HandleStartGame()
        util.Log("HandleStartGame")
        local scene = CCScene:create()
        local layer = CCLayerColor:create(background_color)

        scene:addChild(layer)
        CreateLevelMenu(layer)
        director:replaceScene(scene)
    end

    local start = CCMenuItemLabel:create(start_label)
    start:registerScriptTapHandler(HandleStartGame)

    local menu = CCMenu:createWithItem(start)
    layer:addChild(menu)

    Center(menu, 600)
    director:runWithScene(scene)
end

return scripts
