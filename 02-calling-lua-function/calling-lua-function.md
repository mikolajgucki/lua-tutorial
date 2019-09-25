Calling Lua function from C/C++
====

This tutorial covers calling a Lua function from C/C++. There are 4 functions responsible for that: [`lua_call()`](https://www.lua.org/manual/5.3/manual.html#lua_call), [`lua_pcall()`](https://www.lua.org/manual/5.3/manual.html#lua_pcall),
[`lua_callk()`](https://www.lua.org/manual/5.3/manual.html#lua_callk) and [`lua_pcallk()`](https://www.lua.org/manual/5.3/manual.html#lua_pcallk). The last 2 of them are not covered here as they are related to yielding which is an advanced topic and very rarely used.

Call
----

Calling a Lua function from C/C++ must be done in the following steps:
- The function to call is pushed onto the stack.
- Arguments are pushed in the order they are passed to the function. That is, the first argument is pushed first, then the second argument and so on.
- One of the 4 functions listed above is called. Each of the function removes the Lua function and the arguments from the stack.
- Results (if any) are consumed and popped from the stack. The first result is pushed first, then the second result and so on. This way the last result is at the top of the stack.

The basic function to call a Lua function is [`lua_call()`](https://www.lua.org/manual/5.3/manual.html#lua_call) declared as:

```cpp
void lua_call(lua_State *L,int nargs,int nresults);
```

`nargs` is the number of arguments pushed onto the stack. `nresults` is the number of results to push onto the stack after the function returns. If `nresults` equals `LUA_MULTRET` then Lua pushes all the results. [`lua_call()`](https://www.lua.org/manual/5.3/manual.html#lua_call) makes an unprotected call which means any [error](https://www.lua.org/manual/5.3/manual.html#pdf-error) is propagated upwards. Typically the error reaches the [panic](https://www.lua.org/manual/5.3/manual.html#lua_atpanic) handler and exits the application. See [error handling](https://www.lua.org/manual/5.3/manual.html#4.6) for more information.

Let's consider a simple function which returns its arguments swapped.

```lua
function swap(a,b)
    if not a or not b then
        error('Nil argument')
    end
    return b,a
end
```

Calling `swap()` is straightforward. `swap()` is pushed using the [`lua_getglobal()`](https://www.lua.org/manual/5.3/manual.html#lua_getglobal) function. Strings to be swapped are pushed as arguments. The function is called after which the stack contains two return values (the swapped arguments).

The listing below calls the `swap()` function with arguments `red` and `green`. After the function returns, the results are printed and popped from the stack when the function is called. Notice that last (the second in this situation) argument is at the top of the stack. The same goes for the return values - the last one is at the top of the stack.

```cpp
// push the function
lua_getglobal(L,"swap");

// push arguments
lua_pushstring(L,"red");
lua_pushstring(L,"green");

// call (2 arguments, 2 results)
lua_call(L,2,2);

// get results
const char *first = lua_tostring(L,-2);
const char *second = lua_tostring(L,-1);

// print
std::cout << "swap() returned " << first << " and " << second << "\n";

// pop the arguments
lua_pop(L,2);
```

Protected call
----

If we called `swap()` with one argument using [`lua_call()`](https://www.lua.org/manual/5.3/manual.html#lua_call), the application would crash with an error similar to:
```
PANIC: unprotected error in call to Lua API (main.lua:3: Nil argument)
```

This is due to the fact that [`lua_call()`](https://www.lua.org/manual/5.3/manual.html#lua_call) makes an unprotected call and any error reported by the call is propagated up to the function which made a protected call. If there is no such function in the call stack, then Lua calls the panic function. The default function prints a message similar to the one above and kills the application. A custom panic function can be set using [`lua_atpanic()`](https://www.lua.org/manual/5.3/manual.html#lua_atpanic) declared as:
```cpp
lua_CFunction lua_atpanic(lua_State *L,lua_CFunction panicFunc);
```

If we want to catch the error, we need to make a protected call using  [`lua_pcall()`](https://www.lua.org/manual/5.3/manual.html#lua_pcall):

```cpp
int lua_pcall(lua_State *L,int nargs,int nresults,int handlerIndex);
```

`nargs` and `nresults` have the same meaning as in  [`lua_call()`](https://www.lua.org/manual/5.3/manual.html#lua_call). `handlerIndex` is the stack index of a Lua function called on error. This function is called message handler as it handles the error message (object) which is at the top of the stack. It's a good place to enrich the error message with traceback (described in the next section). [`lua_pcall()`](https://www.lua.org/manual/5.3/manual.html#lua_pcall) returns the call result which can be:
- `LUA_OK` on success,
- `LUA_ERRRUN` on runtime error,
- `LUA_ERRMEM` if it failed to allocate memory (the message handler is not called),
- `LUA_ERRERR` if it failed to call the message handler,
- `LUA_ERRGCMM` if it failed to run a `__gc` metamethod.

Let's create a function which calls `swap` with one argument and therefore always fails.

```lua
function fail()
    swap('abc') -- Fails with message "Nil argument"
end
```

Making a protected call is similar to the unprotected call. The only difference is that we get the call result and error message (if the call failed).

```cpp
// push the function
lua_getglobal(L,"fail");

// call with no arguments, no results
int result = lua_pcall(L,0,0,0);

// check result
if (result == LUA_ERRRUN) {
    // get the error object (message)
    const char *err = lua_tostring(L,-1); // "Nil argument"
    std::cerr << "Error: " << err << "\n";
    
    // pop the error object
    lua_pop(L,1);
    return;
}
else if (result == LUA_ERRMEM) {
    std::cerr << "LUA_ERRMEM\n";
    return;
}
else if (result == LUA_ERRERR) {
    std::cerr << "LUA_ERRERR\n";
    return;
}
else if (result == LUA_ERRGCMM) {
    std::cerr << "LUA_ERRGCMM\n";
    return;
}

// LUA_OK: Process the return values.
```

Traceback
----

It's very handy to get traceback together with an error message. It's possible using the message handler which is called when an error occurs. The error message is at the top of the stack when the handler is called.

We need to change a bit the way we make a protected call. We have to push the message handler before the function to be called and pass the stack index of the handler to [`lua_pcall()`](https://www.lua.org/manual/5.3/manual.html#lua_pcall). The message handler typically replaces the error message with traceback and the error message.

The function getting traceback is [`luaL_traceback()`](https://www.lua.org/manual/5.3/manual.html#luaL_traceback). It creates and pushes the traceback of `L1` onto the stack. It's declared as:

```cpp
void luaL_traceback (lua_State *L,lua_State *L1,const char *msg,int level);
```

Pseudo-code for making a protected call with a message handler could look like:

```cpp
/** */
int messageHandler(lua_State *L) {
    // The error message (object) is at the top of stack at this point.
    // Do whatever necessary with it.
    return 1;
}

// keep the top of the stack to restore it after the call
int top = lua_gettop(L);

// push the message handler
lua_pushcfunction(L,messageHandler);
int handlerIndex = lua_gettop(L);

// push the function to call
// lua_getglobal...

// push arguments
// lua_push...

// call
int result = lua_pcall(L,nargs,nresults,handlerIndex);

// check result
if (result == LUA_ERRRUN) {
    const char *err = lua_tostring(L,-1);
    // ...
    
    // pop the error object
    lua_pop(L,1);
}
else if (result == LUA_ERRMEM) {
    // ...
}
else if (result == LUA_ERRERR) {
    // ...
}
else if (result == LUA_ERRGCMM) {
    // ...
}

// process the return values
// ...

// restore the stack
lua_settop(L,top);
```

Let's get back to the `swap` function. Now we'll now call it with a message handler which enriches the error message with traceback. We'll pass only one argument to `swap` to raise an error.

```cpp
int messageHandler(lua_State *L) {
    const char *errorMsg = lua_tostring(L,-1);
    
    // push traceback
    luaL_traceback(L,L,errorMsg,1);
    
    // remove the original error object
    lua_remove(L,-2);
    
    return 1;
}

// push the error handler
lua_pushcfunction(L,messageHandler);
int handlerIndex = lua_gettop(L);

// push the function
lua_getglobal(L,"swap");    

// push arguments
lua_pushstring(L,"red");
// we're not passing the other argument to raise an error

// call
int result = lua_pcall(L,1,2,handlerIndex);

// check result
if (result == LUA_ERRRUN) {
    // traceback at this point
    const char *err = lua_tostring(L,-1);
    std::cerr << err << "\n";

    // pop the error object
    lua_pop(L,1);        
    return;
}
else if (result == LUA_ERRMEM) {
    std::cerr << "LUA_ERRMEM\n";
    return;
}
else if (result == LUA_ERRERR) {
    std::cerr << "LUA_ERRERR\n";
    return;
}
else if (result == LUA_ERRGCMM) {
    std::cerr << "LUA_ERRGCMM\n";
    return;
}
```

We'll get an error similar to:
```
main.lua:3: Nil argument
stack traceback:
        [C]: in function 'error'
        main.lua:3: in function 'swap'
```

Conclusion
====

Most of the times a Lua function should be called with a message handler. The message handler in turn should enrich the error message with the traceback. This approach lets us get right to the line in the source code which caused the error.
