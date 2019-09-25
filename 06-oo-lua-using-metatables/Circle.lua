local Circle = {}
Circle.__index = Circle

-- superclass
local super = require('Shape')
setmetatable(Circle,{
    __index = super
})

-- Creates new a Circle object.
function Circle.new(name,radius)
    local self = setmetatable({},Circle)
    Circle.construct(self,name,radius)
    return self
end

-- Constructs a Circle object.
function Circle.construct(self,name,radius)
    super.construct(self,name)
    self.radius = radius
end

-- Gets the circle area.
-- This method overrides the method in the Shape class.
function Circle:getArea()
    return math.pi * self.radius * self.radius
end

return Circle