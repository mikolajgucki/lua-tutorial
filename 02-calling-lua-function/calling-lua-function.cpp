#include <iostream>
#include <lua.hpp>

/** */
void callHello(lua_State *L) {
    // push the function
    lua_getglobal(L,"hello");
    if (lua_isnil(L,-1)) {
        std::cerr << "Global function hello not found\n";
        return;
    }

    // call with no arguments
    lua_call(L,0,0);
}

/** */
void callSwap(lua_State *L) {
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
}

/** */
void pcallFail(lua_State *L) {
    // push the function
    lua_getglobal(L,"fail");
    if (lua_isnil(L,-1)) {
        std::cerr << "Global function fail not found\n";
        return;
    }

    // call with no arguments, no results
    int result = lua_pcall(L,0,0,0);

    // check result
    if (result == LUA_ERRRUN) {
        const char *err = lua_tostring(L,-1);
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
}

/** */
int messageHandler(lua_State *L) {
    const char *errorMsg = lua_tostring(L,-1);

    // push traceback
    luaL_traceback(L,L,errorMsg,1);

    // remove the original error object
    lua_remove(L,-2);

    return 1;
}

/** */
void pcallSwap(lua_State *L) {
    int top = lua_gettop(L);

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

    // LUA_OK: process the return values.

    // revert the stack
    lua_settop(L,top);
}

/** */
int main(int argc,char *argv[]) {
    // create Lua state
    lua_State *L = luaL_newstate();
    if (L == (lua_State *)0) {
        std::cerr << "Failed to create Lua state\n";
        return -1;
    }

    // open the standard Lua libraries
    luaL_openlibs(L);

    // load functions
    if (luaL_dofile(L,"calling-lua-function.lua") != 0) {
        const char *error = luaL_checkstring(L,-1);
        printf("Error: %s\n",error);
        return -1;
    }

    callHello(L);
    callSwap(L);
    pcallFail(L);
    pcallSwap(L);

    // destroy the state
    lua_close(L);

    return 0;
}