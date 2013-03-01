-- Level description file.  This file describes a single level.  It
-- is desgined to be referenced from the theme descritions file.
-- It should not pollute the global namespace and should return a
-- single value which is the table describing the level.

local level = {
   goal = { 34, 56 },
   start = { 200, 250 },
   stars = { { 100, 100 }, { 200, 150}, { 300, 200 } },

   objects = {
       -- create four fixed lines in the world forming a box
       { start = { 100, 100 }, finish = { 100, 200 } },
       { start = { 100, 200 }, finish = { 200, 200 } },
       { start = { 200, 200 }, finish = { 200, 100 } },
       { start = { 200, 100 }, finish = { 100, 100 } },

       -- create a dynamic line anchored to the world at a fixed point.
       { start = { 300, 100 }, finish = { 500, 100 }, type = 'dynamic', anchor = { 400, 100 } },
   }
}

return level
