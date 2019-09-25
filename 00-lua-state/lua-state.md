Lua state
====

This part is relatively short as it describes the Lua state and there isn't much about it. A Lua state is an object in which Lua keeps all the necessary data like globals, execution stack and others.

Creating and destroying Lua state
----

There are 2 functions which create a Lua state: [`lua_newstate()`](https://www.lua.org/manual/5.3/manual.html#lua_newstate) coming from Lua itself and [`luaL_newstate()`](https://www.lua.org/manual/5.3/manual.html#luaL_newstate) from the auxiliary library. They are declared as:
```c
lua_State* lua_newstate(lua_Alloc allocFunc, void *userdata);
lua_State *luaL_newstate(void);
```

The former one expects an allocator function ([`lua_Alloc`](https://www.lua.org/manual/5.3/manual.html#lua_Alloc)) to be passed. `userdata` is passed to the allocator every time it's called. The latter function is a more convenient way to create a Lua state as it automatically provides an allocator implementation based on `realloc()`.

A Lua state is destroyed using the function [`lua_close()`](https://www.lua.org/manual/5.3/manual.html#lua_close). It destroys all the Lua objects calling metamethods if appropriate and frees memory. It's declared as:
```c
void lua_close(lua_State *L);
```

A hello world application would look like:
```cpp
#include <iostream>
#include <lua.hpp>

int main(int argc,char *argv[]) {
    // create Lua state
    lua_State *L = luaL_newstate();
    if (L == (lua_State *)0) {
        std::cerr << "Failed to create Lua state\n";
        return -1;
    }

    // open the standard Lua libraries
    luaL_openlibs(L);

    // run sample code
    if (luaL_dostring(L,"print('Hello world!')") != 0) {
        std::cerr << "Failed to do string\n";
        return -1;
    }

    // destroy the state
    lua_close(L);

    return 0;
}
```

Note the function [`luaL_openlibs()`](https://www.lua.org/manual/5.3/manual.html#luaL_openlibs) which is called in the above example. It opens the standard Lua libraries. The libraries provide the function `print` used in the example.

Threads
----

Each thread must have its separate Lua state which is created using [`lua_newthread()`](https://www.lua.org/manual/5.3/manual.html#lua_newthread). The function is declared as:
```c
lua_State *lua_newthread (lua_State *L);
```

A thread state shares the globals with the original state, but it has its own execution stack. Also, a thread state is subject to garbage collection so there is no function to destroy such a state. 
