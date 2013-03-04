-- Level description file.  This file describes a single level.  It
-- is desgined to be referenced from the theme descritions file.
-- It should not pollute the global namespace and should return a
-- single value which is the table describing the level.

local level = {
   goal = { 700, 50 },
   start = { 100, height - 100 },
   stars = { { 200, 490 }, { 200, 250}, { 420, 100 } },

   objects = {
       -- create three ramps for the ball to roll down
       { start = { 20, 450 }, finish = { 550, 400 } },

       { start = { 200, 200}, finish = { 780, 380 } },

       { start = { 20, 240}, finish = { 200, 50 } },

       -- create a box
       { start = { 500, 200 }, finish = { 500, 300 } },
       { start = { 500, 300 }, finish = { 600, 300 } },
       { start = { 600, 300 }, finish = { 600, 200 } },
       { start = { 600, 200 }, finish = { 500, 200 } },

       -- create a dynamic line anchored to the world at a fixed point.
       { start = { 220, 50 }, finish = { 620, 50 }, type = 'dynamic', anchor = { 420, 50 } },
   }
}

return level
