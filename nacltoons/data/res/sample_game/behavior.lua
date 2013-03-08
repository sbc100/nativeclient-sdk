-- Sample game behavior.  This file contains global functions
-- that are called from the C++ game engine when certain events
-- occur.

require 'util'

local num_stars = 3

local game_state = {
   goal_reached = false,
   stars_collected = { },
}

function BeginContact(layer, tag1, tag2)
    local other_tag
    if tag1 == util.tags.BALL then
        other_tag = tag2
    elseif tag2 == util.tags.BALL then
        other_tag = tag1
    else
        -- we are only interested in contacts involving the ball
        return
    end

    -- util.Log('got contact between ball and: ' .. other_tag)
    for i=1,num_stars do
        local star_tag = util.tags.STAR1 + i - 1
        if other_tag == star_tag then
            if game_state.stars_collected[i] ~= true then
                util.Log('star ' .. i .. ' reached')
                game_state.stars_collected[i] = true
                local star = layer:getChildByTag(star_tag)
                local action = CCFadeOut:create(0.5)
                star:runAction(action);
            end
        end
    end
    if game_state.goal_reached ~= true then
        if other_tag == util.tags.GOAL then
            util.Log('goal reached')
            game_state.goal_reached = true
            layer:LevelComplete()
        end
    end
end

function EndContact(layer, tag1, tag2)
    -- util.Log('contact ended: ' .. tag1 .. ' ' .. tag2)
end
