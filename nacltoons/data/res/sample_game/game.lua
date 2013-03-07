-- Game data descrption file.  This file describes a theme and
-- as set of levels.  It should not pollute the global namespace
-- and should return a single value which is the table describing
-- the theme.

local root_dir = 'sample_game'

local theme = {
   background_image = root_dir..'/images/background.png',
   ball_image = root_dir..'/images/ball.png',
   goal_image = root_dir..'/images/goal.png',
   star_image = root_dir..'/images/star.png',
   brush_image = root_dir..'/images/brush.png',
   level_icon = root_dir..'/images/level.png',
   level_icon_selected = root_dir..'/images/level_selected.png',
   levels = {
       dofile(root_dir..'/level1.lua'),
       dofile(root_dir..'/level2.lua'),
       dofile(root_dir..'/level3.lua'),
   }
}

return theme
