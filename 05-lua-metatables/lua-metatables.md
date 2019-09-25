Lua metatables
====

In Lua plain tables have limited set of operations which can be performed on. You can add a value under a key, get a value or you can iterate over key-value pairs. Also, you can get the size of a table of integer keys using the length `#` operator. However, table behavior can be changed using metatables. For example you can change the way a value is added to a table, forbid adding a value or you can define the result of adding two tables.

Every table in Lua can have a metatable. A metatable is a plain table with keys of meaning specific to operation performed on the table. These keys are called metamethods. For example the key `__index` represents the metamethod which reads and returns a value under a given key.

How is a metatable assigned to a table? It's done using the function `setmetatable(table,metatable)`. For example:
```lua
tbl = { 0, 1, 2 }
metatbl = {}
setmetatable(tbl,metatbl)
```
`setmetatable()` returns the first argument which is handy when you want to assign a metatable to an empty table:
```lua
function foo()
    local metatbl = {}
    return setmetatable({},metatbl)
end
```

You can get the metatable assigned to a table using `getmetatable(tbl)`. By default a table doesn't have a metatable assigned:
```lua
tbl = {}
getmetatable(tbl) -- returns nil
```

`__index`
----

The `__index` key represents the metamethod which gets and returns table values. If the Lua interpreter looks up a field in a table which doesn't exist, it will go to the metatable (if assigned to the table) and call the `__index` metamethod. The metamethod can be either a table or function. If `__index` is a table, Lua simply gets the value from this table:
```lua
metatbl = {
    __index = {
        foo = 'bar'
    }
}

tbl = setmetatable({},metatbl)
print(tbl.foo) -- bar
```
In the above snippet the last line prints `bar`. The table `tbl` doesn't have any fields and `metatbl` assigned. Lua goes to the table under `__index` in the metatable and fetch the value under the key `foo` which equals `bar`.

If `__index` is a fuction, Lua simply calls that function with the table being accessed and key as arguments:
```lua
metatbl = {
    __index = function(tbl,key)
        return key:upper()
    end
}

tbl = setmetatable({},metatbl)
print(tbl.foo) -- FOO
print(tbl.bar) -- BAR
```
Again we have an empty table `tbl` with `metatbl` as its metatable. However, `__index` here is a function which simply gets the key being accessed, turns it into its upper case version and returns it. Therefore, the last two lines print the accessed keys in upper case.

`__newindex`
----

The `__newindex` key represents the metamethod which sets values for table. If Lua looks up a field in a table to set its value and there is no such field, it will go to the metatable (if assigned to the table) and call `__newindex`. The metamethod can be either table or function. If `__newindex` is a table, Lua sets the values for this table:
```lua
other = {}
metatbl = {
    __newindex = other
}

tbl = setmetatable({},metatbl)
tbl['foo'] = 'bar'
print(other['foo']) -- bar
```
In the above code the metatable `metatbl` is assigned to the table `tbl`. The `__newindex` points the table `other`. In this situation all values set for `tbl` are actually set for `other`. The last line prints `bar` even though the value was set for `tbl` in the previous line.

If `__newindex` is a function, Lua calls the function with table, key and value as arguments:
```lua
metatbl = {
    __newindex = function(tbl,key,value)
        rawset(tbl,key,value:upper())
    end
}

tbl = setmetatable({},metatbl)
tbl['foo'] = 'bar'
print(tbl['foo']) -- BAR
```
Here `__newindex` is a function which sets the upper case version of the value under the given key. It assumes the value is a string of course.

`rawget` and `rawset`
----
In the above example we could rewrite `__newindex` as:
```lua
metatbl = {
    __newindex = function(tbl,key,value)
        tbl[key] = value:upper()
    end
}
```
What would happen if the example would be run now? The function would enter an infinite loop and cause stack overflow. The value is assigned using the index operator `tbl[key] = ...` which triggers accessing the metatable and calls `__newindex` which tries to assign value using the index operator and so on. It would be the same case for the `__index` metamethod if the index operator would be used in the body of the metamethod.

Lua comes with a solution to the issues. You can access a table without the metatable mechanism. The functions [`rawget`](https://www.lua.org/manual/5.3/manual.html#pdf-rawget) and [`rawset`](https://www.lua.org/manual/5.3/manual.html#pdf-rawset) are responsible for getting and setting values without calling any metamethod. The functions are straightforward:
```lua
-- gets a table value without calling metamethods
rawget(tbl,key)

-- sets a table value without calling metamethods
rawset(tbl,key,value)
```
`rawget` takes table and key as arguments and returns the value. `rawset` takes table, key and value as arguments and sets the value under the key.

`__call`
----

The metamethod `__call` allows to call a table as if it was a function. The `__call` function takes the table as the first argument and other arguments passed while calling the table.

```lua
metatbl = {
    __call = function(tbl,factor)
        local sum = 0
        for _,v in pairs(tbl) do
            if type(v) == 'number' then
                sum = sum + v
            end
        end
        return sum * (factor or 1)
    end
}

tbl = setmetatable({1,2,3,4,'foo'},metatbl)
print(tbl(1)) -- 10
print(tbl(2)) -- 20
```
The metatable in the above example sums all the numbers in a table, multiplies them by a factor (if given) and returns the result.

`__tostring`
----

We can also control the string representation of a table using the `__tostring` metamethod. The function takes a table as the argument and returns its string representation.

```lua
metatbl = {
    __tostring = function(tbl)
        local count = 0
        for k,v in pairs(tbl) do
            count = count + 1
        end
        return string.format('table with %d values',count)
    end
}

tbl = setmetatable({5,3,'foo','bar'},metatbl)
print(tostring(tbl)) -- table with 4 values
```
In the example above the last line prints *table with 4 values*.

`__metatable`
----

The `__metatable` field inside a metatable does two things. First, it forbids overwriting a metatable and such a metatable is called protected metatable. Second, `getmetatable` returns the value assigned to `__metatable`. Consider the example below. In the last line we try to assign `bar` to `tbl` which already has the protected metatable `foo` and therefore the assignment fails.
```lua
foo = {
    __metatable = 'foo'
}

bar = {
}

tbl = setmetatable({},foo)
print(getmetatable(tbl)) -- foo
setmetatable(tbl,bar) -- fails
```

Operators
----

There are metamethods which deal with operators:
- `__unm`: unary minus (`-`)
- `__add`: addition (`+`)
- `__sub`: subtraction (`-`)
- `__mul`: multiplication (`*`)
- `__div`: division (`/`)
- `__mod`: modulo (`%`)
- `__pow`: exponentiation (`^`)
- `__eq`: equality (`=`)
- `__lt`: less than (`<`)
- `__le`: less than or equal to (`<=`)
- `__concat`: concatenation (`..`)

All the above metamethods take the target table (left-hand operand) as the first argument and all of them except unary minus `__unm` take the right-hand operator as the second argument.
