-- Sample game behavior.  This file contains global functions
-- that are called from the C++ game engine when certain events
-- occur:
--   StartGame
--   StartLevel
--   OnTouchBegan(x, y) -- return true to accept touch
--   OnTouchMoved(x, y)
--   OnTouchEnded(x, y)
--   OnContactBegan
--   OnContactEnded
--
-- This file should not alter to global namespace.
--
-- As well as arguments recieved this script has access
-- to the global game variables:
--   game_obj - global state, which persists between level and
--              level reloads but is destroyes on game restart
--              or reload.
--   level_obj - stores level specific state which is destroyed
--               on level restart.

local util = require 'util'
local path = require 'path'
local drawing = require 'drawing'

local handlers = {}
local MENU_DRAW_ORDER = 3

--- Local function for creating a simple menu from text labels
local function CreateMenu(menu_def)
    local menu = CCMenu:create()
    for _, item in ipairs(menu_def.items) do
        local label = CCLabelTTF:create(item[1], menu_def.font, menu_def.font_size)
        local menu_item = CCMenuItemLabel:create(label)
        menu_item:registerScriptTapHandler(item[2])
        menu:addChild(menu_item)
    end
    menu:alignItemsVertically()

    local items = menu:getChildren()
    for i=0,items:count()-1 do
        local item = items:objectAtIndex(i)
        item:setPositionX(item:getContentSize().width/2)
    end
    return menu
end

--- Menu callback
local function HandleRestart()
    GameManager:sharedManager():Restart()
end

--- Menu callback
local function HandleExit()
    CCDirector:sharedDirector():popScene()
end

--- Menu callback
local function ToggleDebug()
    level_obj.layer:ToggleDebug()
end

--- Game behaviour callback.  Called when a level is started/restarted.
-- This function sets up level-specific game state, and adds any UI
-- needed for the level.  In this case we add a menu layer to the scene
-- that is drawn on top of the LevelLayer.
function handlers.StartLevel(level_number)
    util.Log('game.lua: StartLevel: ' .. level_number)

    level_obj.game_state = {
        goal_reached = false,
        stars_collected = { },
    }

    -- Lookup some tags that are used later in the collution code.
    level_obj.ball_tag = level_obj.tag_map['BALL']
    level_obj.goal_tag = level_obj.tag_map['GOAL']
    level_obj.star_tag = level_obj.tag_map['STAR1']

    -- Create a textual menu it its own layer as a sibling of the LevelLayer
    menu_def = {
        font = 'Arial.ttf',
        font_size = 24,
        pos = { 10, 300 },
        items = {
            { 'Restart', HandleRestart },
            { 'Exit', HandleExit },
            { 'Toggle Debug', ToggleDebug } },
    }

    menu = CreateMenu(menu_def)
    local layer = CCLayer:create()
    layer:addChild(menu)
    menu:setPosition(ccp(game_obj.origin.x + menu_def.pos[1],
                         game_obj.origin.y + menu_def.pos[2]))

    local parent = level_obj.layer:getParent()
    parent:addChild(layer, MENU_DRAW_ORDER)
end

--- Forward touch events to default drawing handler.
handlers.OnTouchBegan = drawing.OnTouchBegan

--- Forward touch events to default drawing handler.
handlers.OnTouchMoved = drawing.OnTouchMoved

--- Forward touch events to default drawing handler.
handlers.OnTouchEnded = drawing.OnTouchEnded

--- Game behaviour callback.  Called when two tagged objects start
-- colliding (in the box2d world).
--
-- In this game we check for collisions between the 'BALL' and the
-- 'STAR's that can be collects as well as the 'GOAL'.  Objects
-- are identified using tags.  The numeric tag values are looked up
-- and cached when the level first starts.
function handlers.OnContactBegan(object1, object2)
    util.Log('game.lua: OnContactBegan')

    -- We are only interested in collisions involving the ball
    if object1.tag == level_obj.ball_tag then
        other = object2
    elseif object2.tag == level_obj.ball_tag then
        other = object1
    else
        return
    end

    local state = level_obj.game_state
    for i=1,level_obj.num_stars do
        local star_tag = level_obj.star_tag + i - 1
        if other.tag == star_tag then
            if state.stars_collected[i] ~= true then
                util.Log('star ' .. i .. ' reached')
                state.stars_collected[i] = true
                local star = level_obj.layer:getChildByTag(star_tag)
                local action = CCFadeOut:create(0.5)
                star:runAction(action);
            end
        end
    end

    if state.goal_reached ~= true then
        if other.tag == level_obj.goal_tag then
            util.Log('goal reached')
            state.goal_reached = true

            local function GoalFadeoutComplete(sender)
                level_obj.layer:LevelComplete()
            end

            local goal = level_obj.layer:getChildByTag(level_obj.goal_tag)
            local fadeout = CCFadeOut:create(0.5)
            local fadeout_done = CCCallFuncN:create(GoalFadeoutComplete)
            local seq = CCSequence:createWithTwoActions(fadeout, fadeout_done);
            goal:runAction(seq);
        end
    end
end

--- Game behaviour callback.  Called when two tagged objects stop
-- colliding (in the box2d world).
function handlers.OnContactEnded(object1, object2)
    -- util.Log('game.lua: OnContactEnded')
end

return handlers
