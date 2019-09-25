local Shape = {}
Shape.__index = Shape

-- Constructs a Shape object
function Shape.construct(self,name,color)
    self.name = name
    self.color = color
end

-- Gets the shape name
function Shape:getName()
    return self.name
end

-- Gets the shape area.
-- This function cannot be called as it represents an abstract method.
function Shape:getArea()
    error('Illegal call')
end

return Shape