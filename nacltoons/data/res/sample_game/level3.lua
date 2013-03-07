-- Level description file.  This file describes a single level.  It
-- is desgined to be referenced from the theme descritions file.
-- It should not pollute the global namespace and should return a
-- single value which is the table describing the level.

local level = {
   goal = { 34, 56 },
   start = { 200, 250 },
   stars = { { 100, 100 }, { 200, 150}, { 300, 200 } }
}

return level
