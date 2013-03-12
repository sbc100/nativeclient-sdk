-- Object behaviour script.  This script returns a table contains
-- callback function which are called when event occur on a given
-- object.  As well as arguments recieved this script has access
-- to global game variables:
--   game_obj - global state, which persists between level and
--              level reloads but is destroyed on game restart
--              or reload.
--   level_obj - stores level specific state which is destroyed
--               on level restart.

local scripts = {}

scripts.BeginContact = function(other)
    util.Log('ball-specific contact: ' .. other.tag)
end

return scripts
