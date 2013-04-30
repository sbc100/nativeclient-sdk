-- Sample game behavior.  This file contains global functions
-- that are called from the C++ game engine when certain events
-- occur:
--   StartGame
--   StartLevel
--   OnTouchBegan(x, y, tapcount) -- return true to accept touch
--   OnTouchMoved(x, y, tapcount)
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
local gui = require 'gui'

local handlers = {}
local MENU_DRAW_ORDER = 3
local FONT_NAME = 'Arial.ttf'
local FONT_SIZE = 32
local VELOCITY_ITERATIONS = 8
local POS_ITERATIONS = 1


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

local time_since_last_update = 0

local function PositionTimer(timer)
    local visible_size = CCDirector:sharedDirector():getVisibleSize()
    local xpos = game_obj.origin.x + visible_size.width - timer:getContentSize().width / 1.5
    local ypos = game_obj.origin.y + visible_size.height - timer:getContentSize().height / 1.5
    timer:setPosition(ccp(xpos, ypos))
end

--- Game behaviour callback.  Called every frame with the time delta
-- in seconds since the previous frame.
function handlers.Update(delta)
    -- Update box2d world
    level_obj.world:Step(delta, VELOCITY_ITERATIONS, POS_ITERATIONS)

    -- Check for timeout
    local state = level_obj.game_state
    state.time_remaining = state.time_remaining - delta
    if state.time_remaining < 0 then
        LevelComplete()
    end

    -- Render time remaining
    time_since_last_update = time_since_last_update + delta
    if time_since_last_update > 0.5 then
        time_since_last_update = 0
        level_obj.time_display:setString(string.format("%.0f", state.time_remaining))
        PositionTimer(level_obj.time_display)
    end
end

--- Game behaviour callback.  Called when a level is started/restarted.
-- This function sets up level-specific game state, and adds any UI
-- needed for the level.  In this case we add a menu layer to the scene
-- that is drawn on top of the LevelLayer.
function handlers.StartLevel(level_number)
    util.Log('game.lua: StartLevel: ' .. level_number)
    drawing.handlers.OnTouchBegan = drawn_object_handlers

    -- Initialize game state
    level_obj.game_state = {
        goal_reached = false,
        time_remaining = 30,
        stars_collected = { },
    }

    -- Lookup some tags that are used later in the collution code.
    level_obj.ball_tag = level_obj.tag_map['BALL']
    level_obj.goal_tag = level_obj.tag_map['GOAL']
    level_obj.star_tag = level_obj.tag_map['STAR1']

    -- Create a textual menu as a sibling of the LevelLayer
    menu_def = {
        font_size = 24,
        align = 'Left',
        pos = { 10, 300 },
        items = {
            { name='Restart', callback=HandleRestart },
            { name='Exit', callback=HandleExit },
            { name='Toggle Debug', callback=ToggleDebug }
        },
    }

    menu = gui.CreateMenu(menu_def)
    local parent = level_obj.layer:getParent()
    parent:addChild(menu, MENU_DRAW_ORDER)

    -- Create time display
    level_obj.time_display = CCLabelTTF:create("--", FONT_NAME, FONT_SIZE)
    level_obj.layer:addChild(level_obj.time_display)
    PositionTimer(level_obj.time_display)
end

--- Remove a shape that was previously draw by this drawing module.
function drawing.RemoveShape(tag)
    local sprite = level_obj.brush:getChildByTag(tag)
    -- Remove the box2d body
    sprite = tolua.cast(sprite, "CCPhysicsSprite")
    local body = sprite:getB2Body()
    level_obj.world:DestroyBody(body)
    -- Remove the sprite from the brush (its parent)
    level_obj.brush:removeChild(sprite, true)
end

local last_drawn_shape = nil
local last_draw_time = 0

--- Touch handler to use for all dynamic objects
-- This handler will delete the object when it is double clicked
--
local drawn_object_handlers = {}
function drawn_object_handlers.OnTouchBegan(self, x, y, tapcount)
    if tapcount == 2 then
        -- If there was an object created by the first click of this
        -- double click then remove it now.
        if CCTime:getTime() - last_draw_time < 0.400 then
            -- If the receiving object is that last drawn
            -- object then ignore
            if last_drawn_shape:getTag() == self.tag then
                return false
            end
            drawing.RemoveShape(last_drawn_shape:getTag())
            last_drawn_shape = nil
        end

        util.Log('double-click on drawn object: ' .. self.tag_str)
        -- Cancel delayed drawing
        drawing.RemoveShape(self.tag)

        return true
    end
end

--- Forward touch events to default drawing handler.
function handlers.OnTouchBegan(x, y, tapcount)
    if drawing.IsDrawing() then
        return false
    end

    last_draw_time = CCTime:getTime()
    return drawing.OnTouchBegan(x, y, tapcount)
end

--- Forward touch events to default drawing handler.
function handlers.OnTouchMoved(x, y)
    drawing.OnTouchMoved(x, y)
end

--- Forward touch events to default drawing handler.
function handlers.OnTouchEnded(x, y)
    last_drawn_shape = drawing.OnTouchEnded(x, y)
end

--- Called when two tagged objects start colliding (in the box2d world).
--
-- In this game we check for collisions between the 'BALL' and the
-- 'STAR's that can be collects as well as the 'GOAL'.  Objects
-- are identified using tags.  The numeric tag values are looked up
-- and cached when the level first starts.
function handlers.OnContactBegan(object1, object2)
    -- util.Log('game.lua: OnContactBegan')

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
                LevelComplete()
            end

            local goal = level_obj.layer:getChildByTag(level_obj.goal_tag)
            local fadeout = CCFadeOut:create(0.5)
            local fadeout_done = CCCallFuncN:create(GoalFadeoutComplete)
            local seq = CCSequence:createWithTwoActions(fadeout, fadeout_done);
            goal:runAction(seq);
        end
    end
end

--- Called when two tagged objects stop colliding (in the box2d world).
function handlers.OnContactEnded(object1, object2)
    -- util.Log('game.lua: OnContactEnded')
end

return handlers
