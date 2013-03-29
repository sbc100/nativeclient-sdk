-- Object behaviour script.  This script returns a table containing
-- callback function which are called when events occur on a given
-- object.  It should not pollute the global namespace.
--
-- The following callbacks are available to game objects:
--
--   OnTouchBegan(self, x, y) - return true to accept touch
--   OnTouchMoved(self, x, y)
--   OnTouchEnded(self, x, y)
--   OnContactBegan(self, other)
--   OnContactEnded(self, other)
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

function handlers.OnTouchBegan(self, x, y)
    util.Log('ball touch began')
    return true
end

function handlers.OnTouchEnded(self, x, y)
    util.Log('ball touch ended')
end

function handlers.OnContactBegan(self, other)
    util.Log('ball contact start: ' .. other.tag_str)
end

function handlers.OnContactEnded(self, other)
    util.Log('ball contact ended: ' .. other.tag_str)
end

return handlers
