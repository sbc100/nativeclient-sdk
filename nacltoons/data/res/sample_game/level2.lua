-- Example of a custom level.lua script which can override game
-- lua script.  All this level script does it peform drawing offset
-- from the touch location.

local drawing = require 'drawing'

local handlers = {}

local drawing_offset = 30

function handlers.OnTouchBegan(x, y)
    return drawing.OnTouchBegan(x + drawing_offset, y + drawing_offset)
end

function handlers.OnTouchMoved(x, y)
    drawing.OnTouchMoved(x + drawing_offset, y + drawing_offset)
end

function handlers.OnTouchEnded(x, y)
    drawing.OnTouchEnded(x + drawing_offset, y + drawing_offset)
end

return handlers
