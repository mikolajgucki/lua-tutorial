# Lua tutorial

This [Lua](https://www.lua.org/) tutorial provides a number of lessons on advanced Lua topics. It is a good starting point for integrating Lua in C/C++ applications. It is assumed that you already have Lua knowledge.

It starts with a description of the Lua state which keeps all the necessary information to run Lua code in C/C++. It explains what a Lua stack is which is the key to understanding the Lua C/C++ API. It describes other topics related to the C/C++ api: how to call a Lua function, how to implement a Lua library and how to manipulate Lua tables.

There are also lessons on metatables and how to achieve object oriented techniques in Lua using metatables.

The lessons are:
- [Lua state](00-lua-state/lua-state.md)
- [Lua stack - the key to understanding the Lua C API](01-lua-stack-key-to-understanding-c-api/lua-stack-key-to-understanding-c-api.md)
- [Calling Lua function from C/C++](02-calling-lua-function/calling-lua-function)
- [Implementing a Lua library in C/C++](03-implementing-lua-library-in-cpp/implementing-lua-library-in-cpp.md)
- [Manipulating Lua tables in C/C++](04-manipulating-lua-tables-in-cpp/manipluating-lua-tables-in-cpp.md)
- [Lua metatables](05-lua-metatables/lua-metatables.md)
- [Object-oriented Lua using metatables](06-oo-lua-using-metatables/oo-lua-using-metatables.md)