-- Sample game behavior.  This file contains global functions
-- that are called from the C++ game engine when certain events
-- occur:
--   StartGame
--   StartLevel
--   BeginContact
--   EndContact
--
-- As well as arguments recieved this script has access
-- to global game variables:
--   game_obj - global state, which persists between level and
--              level reloads but is destroyes on game restart
--              or reload.
--   level_obj - stores level specific state which is destroyed
--               on level restart.

require 'util'
require 'path'

local num_stars = 3

local scripts = {}

scripts.StartGame = function()
    util.Log('game.lua: StartGame')
end

scripts.StartLevel = function(level_number)
    util.Log('game.lua: StartLevel: ' .. level_number)
    level_obj.game_state = {
        goal_reached = false,
        stars_collected = { },
    }

    -- Lookup some tags that are used later in the collution code.
    level_obj.ball_tag = level_obj.leveldef.tag_map['BALL']
    level_obj.goal_tag = level_obj.leveldef.tag_map['GOAL']
    level_obj.star_tag = level_obj.leveldef.tag_map['STAR1']
end

scripts.BeginContact = function(object1, object2)
    util.Log('game.lua: BeginContact')

    -- We are only interested in collisions involving the ball
    if object1.tag == level_obj.ball_tag then
        other = object2
    elseif object2.tag == level_obj.ball_tag then
        other = object1
    else
        return
    end

    local state = level_obj.game_state
    for i=1,num_stars do
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

scripts.EndContact = function(tag1, tag2)
    -- util.Log('game.lua: BeginContact')
end

return scripts
