-- Object behaviour script.  This script returns a table containing
-- callback function which are called when events occur on a given
-- object.  It should not pollute the global namespace.
--
-- The following callbacks are available to game objects:
--
--   OnTouchBegan(x, y) - return true to accept touch
--   OnTouchMoved(x, y)
--   OnTouchEnded(x, y)
--   OnContactBegan(other_def)
--   OnContactEnded(other_def)
--
-- As well as arguments recieved this script has access
-- to global game variables:
--   game_obj - global state, which persists between level and
--              level reloads but is destroyed on game restart
--              or reload.
--   level_obj - stores level specific state which is destroyed
--               on level restart.

local handlers = {}

local util = require 'util'

function handlers.OnTouchBegan(other)
    util.Log('ball touch began')
    return true
end

function handlers.OnTouchEnded(other)
    util.Log('ball touch ended')
end

function handlers.OnContactBegan(other)
    util.Log('ball contact start: ' .. other.tag_str)
end

function handlers.OnContactEnded(other)
    util.Log('ball contact ended: ' .. other.tag_str)
end

return handlers
