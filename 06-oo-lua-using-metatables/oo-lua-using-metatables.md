Object-oriented Lua using metatables
====

Lua doesn't provide any object-oriented features directly. However, metatables give us the possibility to write object-oriented code. This tutorial explains how to use metatables so that tables behave like instances of classes (objects).

We'll start with a simple class representing a person. Then we'll move to inheritance and demonstrate it with a simple class hierarchy with shape as the top class and circle as a subclass. If you're not familiar with metatables, make sure to check out our [tutorial](http://www.and-creations.com/2016/05/30/lua-metatables/).

Class
----

The most important thing about classes in Lua is that a **metatable represents a class** while a **table represents an object** instantiated from the class. The metatable is assigned to the table in this situation. Functions set in `__index` represent class methods while values set in the table represent class instance (object) fields.

Let's take a look at the class `Person`.
```lua
-- the metatable which represents the class
local Person = {}
Person.__index = Person

-- Creates a Person object.
function Person.new(firstName,lastName)
    -- create an instance of the class Person
    local self = setmetatable({},Person)
    
    -- initialize the object
    self.firstName = firstName
    self.lastName = lastName
    
    return self
end

-- Formats the full name.
local function formatFullName(self)
    return self.firstName .. ' ' .. self.lastName
end

-- Gets the person full name.
function Person:getFullName()
    -- self is implicitly provided by Lua when using the colon operator
    return formatFullName(self)
end

return Person
```

The most important parts of the code are the first two lines and the line in which the metatable is assigned to an empty table (`setmetatable()`).

The metatable `Person` represents the class. Due to assigning `Person` to `__index` all the functions set in `Person` automatically become the methods of the class `Person`. According to the metatable mechanism if there is no given field in a table, Lua will look up the field in the metatable. This feature allows to implement object and class variables and methods. A field in the table is an object field, a field in the metatable is a class field. The function `getFullName` uses the colon operator which is described later.

Let's take a look at a sample code which uses the class `Person`.
```lua
local Person = require('Person')

local johnDoe = Person.new('John','Doe')
print('Hi! My name is ' .. johnDoe:getFullName()) -- Hi! My name is John Doe
```

In the third line where a sample person is created, not surprisingly we call `new` on the class table to create an instance of the class. The function `new` creates and initializes (constructs) the object. The object is nothing more than an empty table with the `Person` metatable assigned. Any field set for that table becomes an object field which can be either a value or function. 

Colon operator
----

In the next line where the person name is printed, we call the function `getFullName` on the instance table. The function is called using the colon operator which hides the receiver of the operation. The receiver is denoted as `self` in Lua and it is the table on which the function is called. The method can be rewritten using the dot operator as:
```lua
-- Gets the person full name.
function Person.getFullName(self)
    return formatFullName(self)
end
```

The same way we can change the call of the method:
```lua
print('Hi! My name is ' .. johnDoe.getFullName(johnDoe))
```

Both of the ways can be used interchangeably. They have the same meaning. The only difference is that the colon operator hides (must not be included in the argument list) and implicitly provides `self`.

Private methods
----

The Lua local functionality allows us to easily implement private fields of a class. If we keep one class in one Lua file, then everything that's local automatically becomes private to the class inside the file. For example the function `formatFullName` is a private method. 

Inheritance
----

A metatable is a plain table which means that a metatable can have another metatable assigned. This feature lets us implement class inheritance by assigning a metatable which represents a superclass to a metatable which represents a class.

In this situation when a method is called, the Lua interpreter will look up the corresponding function in the object table. If the function doesn't exist, Lua will look up the function in the class metatable and if it doesn't exist in this metatable Lua will look it up in the superclass metatable. In other words, Lua will start looking up the field from the subclass, through superclasses to the top class of the hierarchy. 

Let's start with the example superclass `Shape`.
```lua
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
```

It's similar to the class `Person`. The only difference is that instead of `new` there is the function `construct`. Why is it so? `Shape` in this situation is an abstract class which cannot be instantiated and `construct` is called by subclasses. Note that `construct` takes `self` as argument which means it must be called with an object being constructed.

The example subclass `Circle` is listed below. Subclassing takes place where the metatable `Shape` is assigned to the metatable `Circle`. If a method is called on a `Circle` instance, Lua will look up the corresponding function in the object table, then in the `Circle` class and just then in the `Shape` class. For example Lua would call `getArea()` from the `Circle` class as this method is overridden. Note that if it wouldn't be overridden, `Circle` would be an abstract class and attempt to call `getArea()` would fail with error `Illegal call`.  

```lua
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
```

The superclass is referenced through the local variable `super` which is equal to the superclass metatable. Methods of a superclass can only be called using the dot operator and passing `self` (if it's not a static method). If we used the colon operator, Lua would look up the function in the object table, then in the class table and just then in the superclass table and we want to call the method in the superclass directly.

Creating a `Circle` instance and calling methods on it is straightforward. The function `printArea` takes a shape as argument. It could be an instance of another `Shape` subclass. It would have to implement the method `getArea`.

```lua
local function printArea(shape)
    print(string.format('The area of shape "%s" is %.2f',
        shape:getName(),shape:getArea()))
end

local Circle = require('Circle')
printArea(Circle.new('my circle',10)) -- The area of shape "my circle" is 314.16
```

Conclusion
----

Lua itself doesn't provide any object-oriented features. However, we can create classes, class hierarchy, class instances (objects) and we can override methods using metatables. We must be careful as there is no compile or runtime checking if there are object-oriented errors (like call of an abstract method).

It is also possible to implement multiple inheritance. It just requires a special metatable which combines a number of superclasses. The `__index` metamethod must be a function which picks the superclass which contains the looked up key.

We can create utility functions which make it easier to implements OO in Lua. The below functions are enough to build class hierarchy and create objects. 

```lua
-- Creates a class.
function oo.class()
    local class = {}
    class.__index = class
    
    return class
end

-- Creates a subclass.
function oo.subclass(superclass)
    local subclass = {}
    subclass.__index = subclass
        
    setmetatable(subclass,{
        __index = superclass
    })
       
    return subclass,superclass
end

-- @brief Creates a new object of a class.
function oo.new(class)
    local object = setmetatable({},class)
    return object
end
```
