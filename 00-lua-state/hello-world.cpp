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