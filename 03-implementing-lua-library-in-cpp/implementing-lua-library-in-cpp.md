Implementing a Lua library in C/C++
====

The previous [tutorial](http://www.and-creations.com/2016/09/06/lua-stack-the-key-to-understanding-the-lua-c-api/) explained the Lua stack. It's the fundamental knowledge to start implementing a Lua library in C/C++. You should make sure that you understand the Lua stack 
before carrying on with this tutorial and the Lua C API in general.

We will describe here 2 ways of adding C/C++ functions which can be called from Lua. The first one registers a function as a global. The second one uses the C/C++ version of the [`require`](https://www.lua.org/manual/5.3/manual.html#pdf-require) function.

Note that the header file `lua.hpp` must be included in C++ sources.

C/C++ function
----

Every function in C/C++ which can be called from Lua must be of the type [`lua_CFunction`](https://www.lua.org/manual/5.3/manual.html#lua_CFunction):
```cpp
typedef int (*lua_CFunction)(lua_State *L);
```
Each function of this type takes a pointer to Lua state as argument and returns an integer. The Lua state contains the stack with the function arguments. 

The first arguments is pushed first onto the stack so it's at the bottom of the stack with index 1. The second argument is under index 2, the third argument under index 3 and so on. In other words, the stack index corresponds to the argument index.

The integer returned by the function equals the number of values returned by the Lua function. The return values are pushed onto the Lua stack. After returning from the function Lua discards the arguments and takes only the return values into account so the arguments must be left on the stack as they are.

The number of arguments can be retrieved using the function [`lua_gettop()`](https://www.lua.org/manual/5.3/manual.html#lua_gettop):
```cpp
int lua_gettop(lua_State *L);
```

Registering a function
----

Registering a function as global is straightforward. It's done using the macro [`lua_register()`](https://www.lua.org/manual/5.3/manual.html#lua_register) where `name` is the name of the global and `func` is the function to be registered under the global. [`lua_register()`](https://www.lua.org/manual/5.3/manual.html#lua_register) is defined as:
```cpp
void lua_register(lua_State *L,const char *name,lua_CFunction func);
```

Below is an example which registers a function `write` which simply writes a message to the standard output. In Lua the function can be accessed directly as it's a global. An example usage of `write` will be shown at the end of the tutorial. 

```cpp
#include <iostream>
#include "writer.h"

/** */
static int write(lua_State *L) {
    const char *msg = luaL_checkstring(L,1);
    std::cout << msg << std::endl;
    
    return 0;
}

/** */
void loadWriterLib(lua_State *L) {
    lua_register(L,"write",write);
}
```

```h
#ifndef WRITER_H
#define WRITER_H

#include <lua.hpp>

/** Loads the writer library. */
void loadWriterLib(lua_State *L);

#endif // WRITER_H
```

Arguments
----

Any function in Lua can take any number of arguments of various types. Moreover, it's legal to call the same Lua function with different arguments each time. How can a C/C++ function handle this situation? Lua comes with functions dealing with values on the stack. As arguments are on the stack, then we can use the stack-related functions. The basic functions are explained in the previous  [tutorial](http://www.and-creations.com/2016/09/06/lua-stack-the-key-to-understanding-the-lua-c-api/) about the Lua stack.

There are utility functions which check if a value (argument) is of given type or can be converted to the type and if so returns the value. Otherwise an error is reported. All the functions below behave this way:
- [`luaL_checkinteger()`](https://www.lua.org/manual/5.3/manual.html#luaL_checkinteger) checks if an argument is an integer and returns it as [`lua_Integer`](https://www.lua.org/manual/5.3/manual.html#lua_Integer),
- [`luaL_checknumber()`](https://www.lua.org/manual/5.3/manual.html#luaL_checknumber) checks if an argument is a number and returns it as [`lua_Number`](https://www.lua.org/manual/5.3/manual.html#lua_Number),
- [`luaL_checkstring()`](https://www.lua.org/manual/5.3/manual.html#luaL_checkstring) checks if an argument is a string,
- [`luaL_checkudata()`](https://www.lua.org/manual/5.3/manual.html#luaL_checkudata) checks if an argument is a userdata of a given type name (will be explained in a later tutorial).

The functions are defined as:
```cpp
lua_Integer luaL_checkinteger(lua_State *L,int arg);
lua_Number luaL_checknumber(lua_State *L,int arg);
const char *luaL_checkstring(lua_State *L,int arg);
void *luaL_checkudata(lua_State *L,int arg,const char *typeName);
```

Note that the string returned by [`luaL_checkstring()`](https://www.lua.org/manual/5.3/manual.html#luaL_checkstring) might not be valid after the corresponding value is removed from the stack. It's due to the Lua garbage collection. 

Other check functions that might be handy are [`luaL_checkany()`](https://www.lua.org/manual/5.3/manual.html#luaL_checkany) and [`luaL_checktype()`](https://www.lua.org/manual/5.3/manual.html#luaL_checktype). The former one checks if an argument is of any type including `nil` and the latter one if an argument is of given type. See the description of  [`lua_type()`](https://www.lua.org/manual/5.3/manual.html#lua_type) which lists the types. The functions are defined as:
```cpp
void luaL_checkany(lua_State *L,int arg);
void luaL_checktype(lua_State *L,int arg,int type);
```

Variable number of arguments
----

Any Lua function can take a variable number of arguments. This is also the case for the C/C++ functions. We can check if an argument exists using  [`lua_isnoneornil()`](https://www.lua.org/manual/5.3/manual.html#lua_isnoneornil) defined as:
```cpp
int lua_isnoneornil(lua_State *L,int index);
```

It returns 1 if the value (argument) at the given index is invalid (does not exist) or is nil and 0 otherwise. In other words, it returns 0 if the value is valid.

Let's consider a sample function makes a socket connection. We need a host address and port. Let's assume that the port is optional and a default port is defined. The function then accepts either 1 (host address) or 2 arguments (host address and port). It can be implemented as:
```cpp
/** */
static int connect(lua_State *L) {
    const char *hostAddress = luaL_checkstring(L,1);
    
    int port = DEFAULT_PORT;
    if (lua_isnoneornil(L,2) == 0) {
        port = luaL_checkinteger(L,2);
    }

    // TODO Connect here and possibly return result. 
    
    return 0;
}
```

Return values
----

Values returned to Lua from a C/C++ function are simply pushed onto the Lua stack. The arguments in the stack are discarded by Lua and only the return values remain. Values are pushed using the `lua_push...()` functions described in the previous [tutorial](http://www.and-creations.com/2016/09/06/lua-stack-the-key-to-understanding-the-lua-c-api/).

Loading a library
----

A library implemented in C/C++ is loaded using the C/C++ version of the [`require`](https://www.lua.org/manual/5.3/manual.html#pdf-require) function [`luaL_requiref()`](https://www.lua.org/manual/5.3/manual.html#luaL_requiref):
```cpp
void luaL_requiref(lua_State *L,const char *moduleName,lua_CFunction func,int global);
```

[`luaL_requiref()`](https://www.lua.org/manual/5.3/manual.html#luaL_requiref) calls the function `func` with `moduleName` as argument (on the Lua stack) and stores the call result under `package.loaded[moduleName]` and leaves it on the stack. If `global` is true, it also stores the call result under the global `moduleName`. `func` is a  [`lua_CFunction`](https://www.lua.org/manual/5.3/manual.html#lua_CFunction) and therefore it has to push the call result onto the stack and return 1.  

What should `func` look like to load a library? [`luaL_newlib()`](https://www.lua.org/manual/5.3/manual.html#luaL_newlib) does the job:
```cpp
void luaL_newlib (lua_State *L,const luaL_Reg funcs[]);
```

It's a macro which creates a table and registers all the `funcs` in the table. `funcs` must be an array, not the pointer to the array. [`luaL_Reg`](https://www.lua.org/manual/5.3/manual.html#luaL_Reg) is defined as:
```cpp
typedef struct luaL_Reg {
    const char *name;
    lua_CFunction func;    
} luaL_Reg;
```

`name` is the function name and `func` is the function to be called. Any array of [`luaL_Reg`](https://www.lua.org/manual/5.3/manual.html#luaL_Reg) must be terminated by an entry in which `name` and `func` are null.

The _hello world_ library could look like as listed below. The `world` function would be called in Lua as `hello.world()` as first we must reference the table which contains the library functions.

```cpp
#include <iostream>
#include <lua.hpp>

/** */
static const char *libName = "hello";

/** */
static int world(lua_State *L) {
    std::cout << "Hello World!" << std::endl;
    return 0;
}

/** */
static const struct luaL_Reg helloFuncs[] = {
    {"world",world},
    {0,0}
};

/** */
static int helloRequire(lua_State *L) {
    luaL_newlib(L,helloFuncs);
    return 1;
}

/** */
void loadHelloLib(lua_State *L) {
    luaL_requiref(L,libName,helloRequire,1);
    lua_pop(L,1); // pop the library from the stack
}
```

Reporting errors
----

If you want to report an error, you need to call [`lua_error()`](https://www.lua.org/manual/5.3/manual.html#lua_error):
```cpp
int lua_error(lua_State *L);
```

This function takes the top value in the stack as the error object. It's a string most of the times. [`lua_error()`](https://www.lua.org/manual/5.3/manual.html#lua_error) does a long jump and never returns.

Reporting an error with a string as the error object is straightforward:
```cpp
if (failed) {
    lua_pushstring(L,"Something went wrong");
    lua_error(L);
    // lua_error() never returns and therefore there's no need to return from this function
}
```

Example library
----

The below 2 listings are the source and header files of a sample string library `str`. The library provides functions which check if a string starts (`str.startsWith()`) or ends (`str.endsWith()`) with another string and a function which concatenates a number of strings (`str.concat()`). The function `concat` fails if no argument is provided.

```cpp
#include <string>
#include <cstring>
#include "str.h"

using namespace std;

/** The library name. */
static const char *libName = "str";

/** Checks if a string starts with another string. */
static int startsWith(lua_State *L) {
    const string str(luaL_checkstring(L,1));
    const string prefix(luaL_checkstring(L,2));
    
    int result = 0; // Lua false
    if (str.compare(0,prefix.length(),prefix) == 0) {
        result = 1; // Lua true
    }
    
    lua_pushboolean(L,result);
    return 1;
}

/** Checks if a string ends with another string. */
static int endsWith(lua_State *L) {
    const string str(luaL_checkstring(L,1));
    const string suffix(luaL_checkstring(L,2));
    
    int result = 0; // Lua false
    int offset = str.length() - suffix.length();
    if (str.compare(offset,suffix.length(),suffix) == 0) {
        result = 1; // Lua true
    }
    
    lua_pushboolean(L,result);
    return 1;
}

/** Concatenates a number of strings. */
static int concat(lua_State *L) {
    string result;
    
    int top = lua_gettop(L); // equals the number of arguments
    if (top == 0) {
        lua_pushstring(L,"No strings to concatenate");
        lua_error(L);
        // No need to return as lua_error() does a long jump and never returns.
    }
    
    // iterate over all the arguments
    for (int index = 1; index <= top; index++) {
        const string str = string(luaL_checkstring(L,index));
        result.append(str);
    }
    
    // Lua will make a copy of the string so it's safe to let the result
    // string be destroyed at the end of the function.
    lua_pushstring(L,result.c_str());
    
    return 1;
}

/** The library functions. */
static const struct luaL_Reg strLibFuncs[] = {
    {"startsWith",startsWith},
    {"endsWith",endsWith},
    {"concat",concat},
    {0,0}
};

/** */
static int strLibRequire(lua_State *L) {
    luaL_newlib(L,strLibFuncs);
    return 1;
}

/** */
void loadStrLib(lua_State *L) {
    luaL_requiref(L,libName,strLibRequire,1);
    lua_pop(L,1); // pop the library from the stack
}
```

```h
#ifndef STR_H
#define STR_H

#include <lua.hpp>

/** Loads the string library. */
void loadStrLib(lua_State *L);

#endif // STR_H
```

Let's see how the library `str` and the function `write` can be used. The C++ code below creates a Lua state, loads the libraries and runs a sample Lua file. The Lua file simply calls the functions from the libraries.

```cpp
#include <lua.hpp>

#include "writer.h"
#include "str.h"

/** */
int main(int argc,char *argv[]) {
    // create Lua state
    lua_State *L = luaL_newstate();
    if (L == (lua_State *)0) {
        printf("Failed to create Lua state\n");
        return -1;
    }
    
    // load the libraries
    loadWriterLib(L);
    loadStrLib(L);
    
    // run sample code
    if (luaL_dofile(L,"main.lua") != 0) {
        const char *error = luaL_checkstring(L,-1);
        printf("Error: %s\n",error);
        return -1;
    }    
    
    // destroy the state
    lua_close(L);    
    
    return 0;
}
```

```lua
if str.startsWith('Lua C API','Lua') then
    write('Lua C API starts with Lua')
end
if str.endsWith('Lua C API','API') then
    write('Lua C API ends with API')
end

write('abc...def...ghi = ' .. str.concat('abc','def','ghi'))

str.concat() -- Fails with error: No strings to concatenate
```

Conclusion
----

Creating a library is an easy task and is performed in couple of steps:
1. Implement C/C++ functions of the type [`lua_CFunction`](https://www.lua.org/manual/5.3/manual.html#lua_CFunction) which will comprise the library. In each of these functions:
    1. Get arguments using [`lua_gettop()`](https://www.lua.org/manual/5.3/manual.html#lua_gettop), `luaL_check...`, [`lua_isnoneornil()`](https://www.lua.org/manual/5.3/manual.html#lua_isnoneornil) or any other stack-related functions.
    2. Do whatever the function is supposed to do.
    3. Push result values using the `lua_push...` functions.
    4. Return the number of result values.
2. Create an array with elements of the type [`luaL_Reg`](https://www.lua.org/manual/5.3/manual.html#luaL_Reg) containing the C/C++ functions.
3. Load the library using [`luaL_requiref()`](https://www.lua.org/manual/5.3/manual.html#luaL_requiref) and [`luaL_newlib()`](https://www.lua.org/manual/5.3/manual.html#luaL_newlib).

