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
   levels = {
       dofile(root_dir..'/level1.lua'),
       dofile(root_dir..'/level2.lua'),
   }
}

return theme
