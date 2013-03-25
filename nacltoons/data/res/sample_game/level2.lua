-- Example of a custom level.lua script which can override game
-- lua script.  All this level script does it peform drawing offset
-- from the touch location.

require 'drawing'

local scripts = {}

local drawing_offset = 30

scripts.OnTouchBegan = function(x, y)
    return drawing.OnTouchBegan(x + drawing_offset, y + drawing_offset)
end

scripts.OnTouchMoved = function(x, y)
    drawing.OnTouchMoved(x + drawing_offset, y + drawing_offset)
end

scripts.OnTouchEnded = function(x, y)
    drawing.OnTouchEnded(x + drawing_offset, y + drawing_offset)
end

return scripts
