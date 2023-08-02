-- colour of drawing object
local colour = {255,255,255,255}

-- type, position, size, colour, thickness, filled
local obj  = Drawing.new('Rectangle', {25,25}, {25,25}, colour, 2, true)
local obj2 = Drawing.new('Rectangle', {70,25}, {25,25}, colour, 2, false)

-- print uid's of drawing objects
print(obj, obj2)
