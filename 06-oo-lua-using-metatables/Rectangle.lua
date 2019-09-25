local Rectangle = {}
Rectangle.__index = Rectangle

-- superclass
local super = require('Shape')
setmetatable(Rectangle,{
    __index = super
})

-- Creates a new Rectangle object.
function Rectangle.new(name,width,height)
    local self = setmetatable({},Rectangle)
    Rectangle.construct(self,name,width,height)
    return self
end

-- Constructs a Rectangle object.
function Rectangle.construct(self,id,width,height)
    super.construct(self,id)
    self.width = width
    self.height = height
end

-- Gets the rectangle area.
-- This method overrides the method in the Shape class.
function Rectangle:getArea()
    return self.width * self.height
end

return Rectangle